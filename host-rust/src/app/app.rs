#![feature(proc_macro_hygiene)]

extern crate crypto;
extern crate hex;
extern crate hex_literal;
extern crate reqwest;
extern crate serde;
extern crate zip;

extern crate streaming;

use crypto::aes::cbc_decryptor;
use crypto::aes::KeySize::KeySize128;
use crypto::blockmodes::NoPadding;
use crypto::buffer::{RefReadBuffer, RefWriteBuffer};
use crypto::symmetriccipher::Decryptor;
use std::convert::TryInto;
use std::fs;
use std::io::{Read, Seek, Write};

use streaming::manifest::{Manifest, MANIFEST_SIZE};
use streaming::serialization::{Deserialize, Serialize};
use streaming::speculos::exchange;

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
    R: Seek + std::io::Read,
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

fn zip_writefile<W>(archive: &mut zip::ZipWriter<W>, name: &str, content: &[u8])
where
    W: Seek + std::io::Write,
{
    archive
        .start_file(name, Default::default())
        .expect("failed to create zip entry");
    archive.write_all(content).expect("failed to write zip");
}

fn pages_to_u8(pages: &[Page]) -> Vec<u8> {
    pages.iter().flat_map(|&page| page).collect()
}

fn macs_to_u8(macs: &[Mac]) -> Vec<u8> {
    macs.iter().flat_map(|&mac| mac).collect()
}

fn pages_from_u8(data: &[u8]) -> Vec<Page> {
    assert_eq!(data.len() % PAGE_SIZE, 0);
    data.chunks(PAGE_SIZE)
        .map(|x| x.try_into().expect("invalid page"))
        .collect()
}

fn macs_from_u8(data: &[u8]) -> Vec<[u8; 32]> {
    assert_eq!(data.len() % 32, 0);
    data.chunks(32)
        .map(|x| x.try_into().expect("invalid MAC"))
        .collect()
}

impl App {
    pub fn from_zip(path: &str) -> App {
        let fname = std::path::Path::new(path);
        let file = fs::File::open(&fname).expect("failed to open zip file");
        let mut archive = zip::ZipArchive::new(file).expect("invalid zip file");

        let mut app = App {
            manifest_hsm_signature: zip_readfile(&mut archive, "manifest.hsm.sig").unwrap(),
            code_pages: pages_from_u8(&zip_readfile(&mut archive, "code.bin").unwrap()),
            data_pages: pages_from_u8(&zip_readfile(&mut archive, "data.bin").unwrap()),
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
            app.code_macs = Some(macs_from_u8(
                &zip_readfile(&mut archive, "device/code.mac.bin").unwrap(),
            ));
            app.data_macs = Some(macs_from_u8(
                &zip_readfile(&mut archive, "device/data.mac.bin").unwrap(),
            ));
        }

        app
    }

    pub fn to_zip(&self, path: &str) {
        let fname = std::path::Path::new(path);
        let file = fs::File::create(&fname).expect("failed to create zip file");
        let mut archive = zip::ZipWriter::new(file);
        zip_writefile(&mut archive, "manifest.bin", &self.manifest);
        zip_writefile(
            &mut archive,
            "manifest.hsm.sig",
            &self.manifest_hsm_signature,
        );
        zip_writefile(&mut archive, "code.bin", &pages_to_u8(&self.code_pages));
        zip_writefile(&mut archive, "data.bin", &pages_to_u8(&self.data_pages));

        if self.manifest_device_signature.is_some() {
            archive
                .add_directory("device/", Default::default())
                .expect("failed to create zip directory");
            zip_writefile(
                &mut archive,
                "device/manifest.device.sig",
                self.manifest_device_signature.as_ref().unwrap(),
            );
            zip_writefile(
                &mut archive,
                "device/code.mac.bin",
                &macs_to_u8(self.code_macs.as_ref().unwrap()),
            );
            zip_writefile(
                &mut archive,
                "device/data.mac.bin",
                &macs_to_u8(self.data_macs.as_ref().unwrap()),
            );
            zip_writefile(
                &mut archive,
                "device/device.pubkey",
                self.device_pubkey.as_ref().unwrap(),
            );
        }
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
        .enumerate()
        .map(|(i, &page)| {
            let (status, data) = exchange(0x01, &page[1..], None, Some(page[0]), Some(0x34));
            assert_eq!(status, 0x6802); // REQUEST_APP_HMAC
            println!("{} {:x} {}", i, status, hex::encode(&data));
            let mac: Mac = data.try_into().expect("invalid MAC size");

            let (status, data) = exchange(0x01, &[0u8; 0], None, None, None);
            if last && i == pages.len() - 1 {
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

#[repr(C, packed)]
struct SignatureReq {
    manifest: [u8; MANIFEST_SIZE],
    signature: [u8; 72],
    size: u8,
}

impl Serialize for SignatureReq {}

#[repr(C, packed)]
struct SignatureRes {
    aes_key: [u8; 32],
    signature: [u8; 72],
    size: u8,
}

impl Deserialize for SignatureRes {}

fn decrypt_macs(aes: &mut Box<dyn Decryptor>, enc_macs: &[Mac]) -> Vec<Mac> {
    enc_macs
        .iter()
        .map(|mac| {
            let mut buffer = [0u8; 32];
            let mut read_buffer = RefReadBuffer::new(&mac[..]);
            let mut write_buffer = RefWriteBuffer::new(&mut buffer);
            aes.decrypt(&mut read_buffer, &mut write_buffer, false)
                .unwrap();
            buffer
        })
        .collect()
}

fn device_sign_app(app: &mut App) {
    let device_pubkey = get_pubkey(app);
    let mut signature = [0u8; 72];
    let size = app.manifest_hsm_signature.len();
    signature[..size].copy_from_slice(&app.manifest_hsm_signature);

    let req = SignatureReq {
        manifest: app.manifest,
        signature,
        size: size.try_into().unwrap(),
    };

    let (status, _) = exchange(0x11, &req.to_vec(), None, None, Some(0x34));
    assert_eq!(status, 0x6801); // REQUEST_APP_PAGE

    let (code_macs, _) = get_encrypted_macs(&app.code_pages, false);
    let (data_macs, apdu_data) = get_encrypted_macs(&app.data_pages, true);

    let res = SignatureRes::from_bytes(&apdu_data);

    let iv: [u8; 16] = [0; 16];
    let mut aes = cbc_decryptor(KeySize128, &res.aes_key, &iv, NoPadding);

    app.code_macs = Some(decrypt_macs(&mut aes, &code_macs));
    app.data_macs = Some(decrypt_macs(&mut aes, &data_macs));
    app.manifest_device_signature = Some(res.signature[..res.size as usize].to_vec());
    app.device_pubkey = Some(device_pubkey.to_vec());

    app.to_zip("/tmp/app.signed.zip");
}

pub fn main() {
    let mut app = App::from_zip("/tmp/app.zip");
    let pubkey = get_pubkey(&app);
    println!("{:?}", pubkey);
    device_sign_app(&mut app);
}
