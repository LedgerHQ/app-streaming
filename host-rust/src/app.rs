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
use std::mem;

use manifest::{Manifest, MANIFEST_SIZE};
use speculos::exchange;

const PAGE_SIZE: usize = 256;

type Page = [u8; PAGE_SIZE];
type Mac = [u8; 32];

struct App {
    code_pages: Vec<Page>,
    data_pages: Vec<Page>,

    code_macs: Option<Vec<Mac>>,
    data_macs: Option<Vec<Mac>>,

    device_pubkey: Option<Vec<u8>>,

    manifest: [u8; MANIFEST_SIZE],
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
        file.read_to_end(&mut buffer)
            .expect("failed to read zip entry");
        Some(buffer)
    } else {
        None
    }
}

impl App {
    fn pages_to_list(data: &[u8]) -> Vec<Page> {
        assert_eq!(data.len() % PAGE_SIZE, 0);

        data.chunks(PAGE_SIZE)
            .map(|x| x.try_into().expect("invalid page"))
            .collect()
    }

    fn macs_to_list(data: &[u8]) -> Vec<[u8; 32]> {
        assert_eq!(data.len() % 32, 0);

        data.chunks(32)
            .map(|x| x.try_into().expect("invalid MAC"))
            .collect()
    }

    pub fn from_zip(path: &str) -> App {
        let fname = std::path::Path::new(path);
        let file = fs::File::open(&fname).expect("failed to open zip file");
        let mut archive = zip::ZipArchive::new(file).expect("invalid zip file");

        let mut app = App {
            manifest_hsm_signature: zip_readfile(&mut archive, "manifest.hsm.sig").unwrap(),
            code_pages: App::pages_to_list(&zip_readfile(&mut archive, "code.bin").unwrap()),
            data_pages: App::pages_to_list(&zip_readfile(&mut archive, "data.bin").unwrap()),
            manifest_device_signature: zip_readfile(&mut archive, "device/manifest.device.sig"),
            code_macs: None,
            data_macs: None,
            device_pubkey: None,
            manifest: [0u8; MANIFEST_SIZE],
        };

        app.manifest
            .copy_from_slice(&zip_readfile(&mut archive, "manifest.bin").unwrap());
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
    data.try_into().expect("invalid public key size")
}

fn get_encrypted_macs(pages: &[Page], last: bool) -> (Vec<Mac>, Vec<u8>) {
    let mut apdu_data = Vec::new();
    let macs = pages
        .iter()
        .map(|&page| {
            let (status, data) = exchange(0x01, &page[1..], None, Some(page[0]), Some(0x34));
            assert_eq!(status, 0x6802); // REQUEST_APP_HMAC
            println!("{:x} {}", status, hex::encode(&data));
            let mac: Mac = data.try_into().expect("invalid MAC size");

            let (status, data) = exchange(0x01, &[0u8; 0], None, None, None);
            if last {
                apdu_data = data;
                assert_eq!(status, 0x9000);
            } else {
                assert_eq!(status, 0x6801); // REQUEST_APP_PAGE
            }
            mac
        })
        .collect();

    (macs, apdu_data)
}

unsafe fn any_as_u8_slice<T: Sized>(p: &T) -> &[u8] {
    ::std::slice::from_raw_parts((p as *const T) as *const u8, ::std::mem::size_of::<T>())
}

#[repr(C)]
struct SignatureReq {
    manifest: [u8; MANIFEST_SIZE],
    signature: [u8; 72],
    size: u8,
}

impl SignatureReq {
    pub fn to_vec(&self) -> Vec<u8> {
        unsafe { any_as_u8_slice(self).to_vec() }
    }
}

struct SignatureRes<'a> {
    aes_key: &'a [u8; 32],
    signature: &'a [u8; 72],
    size: u8,
}

const SIGNATURE_RES_SIZE: usize = mem::size_of::<SignatureRes>();

impl<'a> SignatureRes<'a> {
    pub fn from_bytes(data: &[u8]) -> Self {
        let mut buffer = [0u8; mem::size_of::<Self>()];
        buffer.copy_from_slice(data);
        unsafe { std::mem::transmute(buffer) }
    }
}

fn device_sign_app(app: &mut App) {
    let device_pubkey = get_pubkey(app);
    let mut signature = [0u8; 72];
    let size = app.manifest_hsm_signature.len();
    signature[..size].copy_from_slice(&app.manifest_hsm_signature);

    let req = SignatureReq {
        manifest: app.manifest,
        signature: signature,
        size: size.try_into().unwrap(),
    };

    let (status, _) = exchange(0x11, &req.to_vec(), None, None, Some(0x34));
    assert_eq!(status, 0x6801); // REQUEST_APP_PAGE

    let (code_macs, _) = get_encrypted_macs(&app.code_pages, false);
    let (data_macs, apdu_data) = get_encrypted_macs(&app.data_pages, true);

    let res = SignatureRes::from_bytes(&apdu_data);
}

pub fn main() {
    let mut app = App::from_zip("/tmp/app.zip");
    let pubkey = get_pubkey(&app);
    println!("{:?}", pubkey);
    device_sign_app(&mut app);
}
