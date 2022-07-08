#![feature(proc_macro_hygiene)]

extern crate crypto;
extern crate hex;
extern crate hex_literal;
extern crate reqwest;
extern crate serde;
extern crate zip;

mod manifest;
mod merkletree;
mod speculos;

use std::convert::TryInto;
use std::fs;
use std::io::{Read, Seek};

use manifest::Manifest;
use speculos::exchange;

const PAGE_SIZE: usize = 256;

type Page = [u8; PAGE_SIZE];
type Mac = [u8; 32];

#[derive(Default)]
struct App {
    code_pages: Vec<Page>,
    data_pages: Vec<Page>,

    code_macs: Option<Vec<Mac>>,
    data_macs: Option<Vec<Mac>>,

    device_pubkey: Option<Vec<u8>>,

    manifest: Vec<u8>,
    manifest_hsm_signature: Vec<u8>,
    manifest_device_signature: Option<Vec<u8>>,
}

fn zip_readfile<R>(archive: &mut zip::ZipArchive<R>, name: &str) -> Option<Vec<u8>>
where
    R: Seek,
    R: std::io::Read,
{
    let mut buffer = Vec::new();
    if let Ok(mut file) = archive.by_name(name) {
        file.read_to_end(&mut buffer).unwrap();
        Some(buffer)
    } else {
        None
    }
}

impl App {
    fn pages_to_list(data: &[u8]) -> Vec<Page> {
        assert_eq!(data.len() % PAGE_SIZE, 0);

        data.chunks(PAGE_SIZE)
            .map(|x| x.try_into().unwrap())
            .collect()
    }

    fn macs_to_list(data: &[u8]) -> Vec<[u8; 32]> {
        assert_eq!(data.len() % 32, 0);

        data.chunks(32).map(|x| x.try_into().unwrap()).collect()
    }

    pub fn from_zip(path: &str) -> App {
        let fname = std::path::Path::new(path);
        let file = fs::File::open(&fname).unwrap();
        let mut archive = zip::ZipArchive::new(file).unwrap();

        let mut app = App {
            manifest: zip_readfile(&mut archive, "manifest.bin").unwrap(),
            manifest_hsm_signature: zip_readfile(&mut archive, "manifest.hsm.sig").unwrap(),
            code_pages: App::pages_to_list(&zip_readfile(&mut archive, "code.bin").unwrap()),
            manifest_device_signature: zip_readfile(&mut archive, "device/manifest.device.sig"),
            ..Default::default()
        };

        if app.manifest_device_signature.is_some() {
            app.device_pubkey = Some(zip_readfile(&mut archive, "device/device.pubkey").unwrap());
            app.code_macs = Some(App::macs_to_list(
                &zip_readfile(&mut archive, "device/code.mac.bin").unwrap(),
            ));
            app.data_macs = Some(App::macs_to_list(
                &zip_readfile(&mut archive, "device/data.mac.bin").unwrap(),
            ));
        }

        app
    }
}

fn get_pubkey(app: &App) -> [u8; 65] {
    let app_hash = &Manifest::from_bytes(&app.manifest).app_hash;
    let (status, data) = exchange(0x10, app_hash, None, None, Some(0x34));
    assert_eq!(status, 0x9000);
    data.try_into().unwrap()
}

fn get_encrypted_macs(pages: &[Page]) -> Vec<Mac> {
    pages
        .iter()
        .map(|&page| {
            let (status, data) = exchange(0x01, &page[1..], None, Some(page[0]), Some(0x34));
            assert_eq!(status, 0x6802); // REQUEST_APP_HMAC
            let mac: Mac = data.try_into().unwrap();
            let (_status, _) = exchange(0x01, &[0u8; 0], None, None, None);
            // TODO: check status
            mac
        })
        .collect()
}

fn device_sign_app(app: &mut App) {
    let device_pubkey = get_pubkey(app);
    let signature = &app.manifest_hsm_signature;

    let size = app.manifest.len();
    let mut data = vec![0; size + 72 + 1];
    data[..size].copy_from_slice(&app.manifest);
    data[size..size + signature.len()].copy_from_slice(signature);
    data[size + 72] = signature.len().try_into().unwrap();

    let (status, _) = exchange(0x11, &data, None, None, Some(0x34));
    assert_eq!(status, 0x6801); // REQUEST_APP_PAGE

    let code_macs = get_encrypted_macs(&app.code_pages);
    let data_macs = get_encrypted_macs(&app.data_pages);
}

pub fn main() {
    let mut app = App::from_zip("/tmp/app.zip");
    let pubkey = get_pubkey(&app);
    println!("{:?}", pubkey);
    device_sign_app(&mut app);
}
