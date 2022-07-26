use std::collections::HashMap;
use std::iter::zip;

use app::App;
use comm::{Comm, Status};
use manifest;
use merkletree::MerkleTree;
use serialization::{Deserialize, Serialize};

const PAGE_SIZE: usize = 256;
const PAGE_MASK: u32 = 0xffffff00;
const PAGE_MASK_INVERT: u32 = 0x000000ff;

type PageData = [u8; PAGE_SIZE];
type Mac = [u8; 32];
type Addr = u32;

struct Page {
    data: PageData,
    mac: Mac,
    iv: u32,
    read_only: bool,
}

impl Page {
    pub fn new(data: &PageData, mac: &Mac, iv: u32, read_only: bool) -> Self {
        Page {
            data: *data,
            mac: *mac,
            iv: iv,
            read_only,
        }
    }

    pub fn update(&mut self, data: &PageData, mac: &Mac, iv: u32) {
        assert!(self.read_only);
        self.data = *data;
        self.mac = *mac;
        self.iv = iv;
    }
}

struct Buffer<'a> {
    counter: usize,
    index: usize,
    data: &'a [u8],
    received: bool,
}

#[repr(C, packed)]
struct ReadReq {
    addr: Addr,
}

impl Deserialize for ReadReq {}

#[repr(C, packed)]
struct WriteReq1 {
    page_data: PageData,
}

#[repr(C, packed)]
struct WriteReq2 {
    addr: Addr,
    iv: u32,
    mac: Mac,
}

impl Deserialize for WriteReq1 {}
impl Deserialize for WriteReq2 {}

#[repr(C, packed)]
struct ReadRes {
    iv: u32,
    mac: Mac,
}

impl Serialize for ReadRes {}

#[repr(C, packed)]
struct RecvReq {
    counter: u32,
    maxsize: u16,
}

impl Deserialize for RecvReq {}

pub struct Stream {
    pages: HashMap<Addr, Page>,
    app: App,
    manifest: manifest::Manifest,
    initialized: bool,
    comm: Comm,
    merkletree: MerkleTree,
}

impl Stream {
    fn write_page(&mut self, addr: Addr, data: &PageData, mac: &Mac, iv: u32) {
        assert_eq!(addr & PAGE_MASK_INVERT, 0);

        if let Some(page) = self.pages.get_mut(&addr) {
            page.update(data, mac, iv);
            self.merkletree.update(addr, iv);
        } else {
            let page = Page::new(data, mac, iv, false);
            self.pages.insert(addr, page);
            self.merkletree.insert(addr, iv);
        }
    }

    fn get_page(&self, addr: u32) -> &Page {
        assert_eq!(addr & PAGE_MASK_INVERT, 0);
        self.pages.get(&addr).expect("failed to get page")
    }

    pub fn new(path: &str, comm: Comm) -> Self {
        let mut pages = HashMap::new();
        let app = App::from_zip(path);
        let manifest = manifest::Manifest::from_bytes(&app.manifest);
        let mut merkletree = MerkleTree::new();

        let mut addr = manifest.code_start;
        zip::<&Vec<PageData>, &Vec<Mac>>(app.code_pages.as_ref(), app.code_macs.as_ref().unwrap())
            .for_each(|(data, mac)| {
                let page = Page::new(data, mac, 0, true);
                // Since this pages are read-only, don't call write_page() to avoid
                // inserting them in the merkle tree.
                pages.insert(addr, page);
                addr += PAGE_SIZE as u32;
            });

        let mut addr = manifest.data_start;
        zip::<&Vec<PageData>, &Vec<Mac>>(app.data_pages.as_ref(), app.data_macs.as_ref().unwrap())
            .for_each(|(data, mac)| {
                // The IV is set to 0. It allows the VM to tell which key should be
                // used for decryption and HMAC verification.
                let page = Page::new(data, mac, 0, false);
                pages.insert(addr, page);
                merkletree.insert(addr, 0);
                addr += PAGE_SIZE as u32;
            });

        Self {
            pages,
            app,
            manifest,
            initialized: false,
            comm,
            merkletree,
        }
    }

    fn init(&mut self) -> (Status, Vec<u8>) {
        assert!(!self.initialized);

        let signature = self
            .app
            .manifest_device_signature
            .as_ref()
            .expect("app isn't signed");
        let (status, _data) = self.comm.exchange(0x00, signature, None, None, None);
        assert_eq!(status, Status::RequestManifest);

        self.initialized = true;

        // Pad with 3 bytes because of alignment.
        let manifest = self.manifest.to_vec();
        let mut data = vec![0; 3 + manifest.len()];
        data[3..].copy_from_slice(&manifest);

        self.comm.exchange(0x00, &data, None, None, None)
    }

    fn handle_read_access(&self, data: &[u8]) -> (Status, Vec<u8>) {
        // retrieve the encrypted page associated to the given address
        let req = ReadReq::from_bytes(data);
        let page = self.get_page(req.addr);

        let addr = req.addr;
        println!("read access: {:x}", addr);

        // send the page data
        let (status, _data) =
            self.comm
                .exchange(0x01, &page.data[1..], None, Some(page.data[0]), None);
        assert_eq!(status, Status::RequestHmac);

        // send IV and Mac
        let res = ReadRes {
            iv: page.iv,
            mac: page.mac,
        };
        let (status, data) = self.comm.exchange(0x02, &res.to_vec(), None, None, None);

        // send merkle proof
        if !page.read_only {
            assert_eq!(status, Status::RequestProof);
            let (entry, proof) = self.merkletree.get_proof(req.addr);
            assert_eq!((entry.addr, entry.counter), (req.addr, page.iv));
            // TODO: handle larger proofs
            assert!(proof.len() <= 250);
            self.comm.exchange(0x03, &proof, None, None, None)
        } else {
            (status, data)
        }
    }

    fn handle_write_access(&mut self, data: &[u8]) -> (Status, Vec<u8>) {
        let req1 = WriteReq1::from_bytes(data);

        // receive addr, iv and mac
        let (status, data) = self.comm.exchange(0x01, &[0u8; 0], None, None, None);
        assert_eq!(status, Status::CommitHmac);
        let req = WriteReq2::from_bytes(&data);
        assert_eq!(req.addr & PAGE_MASK_INVERT, 0);

        // temporary variables for unaligned references
        let (addr, iv) = (req.addr, req.iv);
        println!("write access: {:x} {:x} {}", addr, iv, hex::encode(req.mac));

        // commit page and send merkle proof
        let (_entry, proof) = if self.merkletree.has_addr(req.addr) {
            let (entry, proof) = self.merkletree.get_proof(req.addr);
            assert_eq!((entry.addr, entry.counter + 1), (req.addr, req.iv));
            (entry, proof)
        } else {
            self.merkletree.get_proof_of_last_entry()
        };

        self.write_page(req.addr, &req1.page_data, &req.mac, req.iv);

        assert!(proof.len() <= 250);

        self.comm.exchange(0x02, &proof, None, None, None)
    }

    fn handle_recv_buffer(&self, data: &[u8], buffer: &mut Buffer) -> (Status, Vec<u8>) {
        // ensure the app doesn't call xrecv() twice
        assert!(!buffer.received);

        let req = RecvReq::from_bytes(data);

        let counter = req.counter as usize;
        let maxsize = req.maxsize as usize;
        assert_eq!(counter, buffer.counter);

        println!("recv buffer: counter: {}, maxsize: {}", counter, maxsize);

        let size = if maxsize > buffer.data.len() - buffer.index {
            buffer.data.len() - buffer.index
        } else {
            maxsize
        };

        let buf = &buffer.data[buffer.index..buffer.index + size];
        buffer.index += size;
        buffer.counter += 1;

        let last = if buffer.index == buffer.data.len() {
            buffer.received = true;
            0x01
        } else {
            0x00
        };

        // the first byte is in p2
        let (p2, data) = if buf.len() == 0 {
            (0x00 as u8, [0u8; 0].as_slice())
        } else {
            (buf[0], &buf[1..])
        };

        self.comm.exchange(0x00, data, Some(last), Some(p2), None)
    }

    pub fn exchange(&mut self, recv_buffer: &[u8]) -> Option<Vec<u8>> {
        let (mut status, mut data) = if !self.initialized {
            self.init()
        } else {
            // resume execution after previous exchange call
            self.comm.exchange(0x00, &[0u8; 0], None, None, None)
        };

        let mut rbuffer = Buffer {
            counter: 0,
            index: 0,
            data: recv_buffer,
            received: false,
        };
        loop {
            match status as Status {
                Status::RequestPage => {
                    (status, data) = self.handle_read_access(&data);
                }
                Status::CommitPage => {
                    (status, data) = self.handle_write_access(&data);
                }
                Status::RecvBuffer => {
                    (status, data) = self.handle_recv_buffer(&data, &mut rbuffer);
                }
                _ => {
                    panic!("TODO");
                }
            }
        }

        None
    }
}
