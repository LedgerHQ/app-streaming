use std::collections::HashMap;
use std::iter::zip;

use app::App;
use comm::Comm;
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
    pub fn new(data: &PageData, mac: &Mac, read_only: bool) -> Self {
        Page {
            data: *data,
            mac: *mac,
            iv: 0,
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

struct Stream<'a> {
    pages: HashMap<Addr, Page>,
    app: App,
    manifest: manifest::Manifest,
    initialized: bool,
    comm: Comm<'a>,
    merkletree: MerkleTree,
}

#[repr(C, packed)]
struct ReadReq {
    addr: u32,
}

impl Deserialize for ReadReq {}

#[repr(C, packed)]
struct ReadRes {
    iv: u32,
    mac: [u8; 32],
}

impl Serialize for ReadRes {}

impl<'a> Stream<'a> {
    /*fn write_page(addr: Addr, data: &PageData, mac: &Mac, iv: u32) {
            /*if let Some(page) = pages.get_mut(addr) {
                    page.update(&data, &mac, 0);
                    merkletree.update();
                } else {
            }*/
        contains_key
    }*/

    pub fn new(path: &str, comm: Comm<'a>) -> Self {
        let mut pages = HashMap::new();
        let app = App::from_zip(path);
        let manifest = manifest::Manifest::from_bytes(&app.manifest);
        let mut merkletree = MerkleTree::new();

        let mut addr = manifest.code_start;
        zip::<&Vec<PageData>, &Vec<Mac>>(app.code_pages.as_ref(), app.code_macs.as_ref().unwrap())
            .for_each(|(data, mac)| {
                let page = Page::new(data, mac, true);
                // Since this pages are read-only, don't call _write_page() to avoid
                // inserting them in the merkle tree.
                pages.insert(addr, page);
                addr += PAGE_SIZE as u32;
            });

        let mut addr = manifest.data_start;
        zip::<&Vec<PageData>, &Vec<Mac>>(app.data_pages.as_ref(), app.data_macs.as_ref().unwrap())
            .for_each(|(data, mac)| {
                // The IV is set to 0. It allows the VM to tell which key should be
                // used for decryption and HMAC verification.
                let page = Page::new(data, mac, false);
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

    pub fn init_app(&mut self) -> (u16, Vec<u8>) {
        assert!(!self.initialized);

        let signature = self
            .app
            .manifest_device_signature
            .as_ref()
            .expect("app isn't signed");
        let (status, _data) = self.comm.exchange(0x00, signature, None, None, None);
        assert_eq!(status, 0x6701); // REQUEST_MANIFEST

        self.initialized = true;

        // Pad with 3 bytes because of alignment.
        let manifest = self.manifest.to_vec();
        let mut data = vec![0; 3 + manifest.len()];
        data[3..].copy_from_slice(&manifest);

        self.comm.exchange(0x00, &data, None, None, None)
    }

    fn get_page(&self, addr: u32) -> &Page {
        assert_eq!(addr & PAGE_MASK_INVERT, 0);
        self.pages.get(&addr).expect("failed to get page")
    }

    pub fn handle_read_access(&mut self, data: &[u8]) -> (u16, Vec<u8>) {
        // retrieve the encrypted page associated to the given address
        let req = ReadReq::from_bytes(data);
        let page = self.get_page(req.addr);

        // send the page data
        let (status, _data) =
            self.comm
                .exchange(0x01, &page.data[1..], None, Some(page.data[0]), None);
        assert_eq!(status, 0x6102); // REQUEST_HMAC

        // send IV and Mac
        let res = ReadRes {
            iv: page.iv,
            mac: page.mac,
        };
        let (status, data) = self.comm.exchange(0x02, &res.to_vec(), None, None, None);

        // send merkle proof
        if !page.read_only {
            assert_eq!(status, 0x6103); // REQUEST_PROOF
            let (entry, proof) = self.merkletree.get_proof(req.addr);
            assert_eq!((entry.addr, entry.counter), (req.addr, page.iv));
            // TODO: handle larger proofs
            assert!(proof.len() <= 250);
            self.comm.exchange(0x03, &proof, None, None, None)
        } else {
            (status, data)
        }
    }
}
