use std::collections::HashMap;
use std::iter::zip;

use app::App;
use comm::Comm;
use manifest;
use merkletree::MerkleTree;
use serialization::Deserialize;

const PAGE_SIZE: usize = 256;

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
}

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
                (data, mac, true);
                // The IV is set to 0. It allows the VM to tell which key should be
                // used for decryption and HMAC verification.
                let page = Page::new(data, mac, false);
                merkletree.insert(addr, 0);
                addr += PAGE_SIZE as u32;
            });

        Self {
            pages,
            app,
            manifest,
            initialized: false,
            comm,
        }
    }

    pub fn init_app(&mut self) /*-> (u16, Vec<u8>)*/ {
        assert!(!self.initialized);

        let signature = self
            .app
            .manifest_device_signature
            .as_ref()
            .expect("app isn't signed");
        let (status, data) = self.comm.exchange(0x00, signature, None, None, None);
        assert_eq!(status, 0x6701); // REQUEST_MANIFEST

        self.initialized = true;
    }
}
