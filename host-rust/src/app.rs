use std::fs;
use std::io;
use std::io::{Bytes, Read, Seek};
use std::iter::zip;
use zip::read::ZipFile;
use zip::result::ZipResult;

const PAGE_SIZE: usize = 256;

type Page = [u8; PAGE_SIZE];
type Mac = [u8; 32];

struct App {
    code_pages: Vec<Page>,
    data_pages: Vec<Page>,

    code_macs: Option<Mac>,
    data_macs: Option<Mac>,

    device_pubkey: Option<Vec<u8>>,

    manifest: Vec<u8>,
    manifest_hsm_signature: Vec<u8>,
    manifest_device_signature: Vec<u8>,
}

fn zip_readfile<'a, R>(
    archive: &'a mut zip::ZipArchive<R>,
    name: &str,
) -> Option<Bytes<ZipFile<'a>>>
where
    R: Seek,
    R: std::io::Read,
{
    if let Ok(mut file) = archive.by_name(name) {
        Some(file.bytes())
    } else {
        None
    }
}

impl App {
    pub fn from_zip(path: &str) /*-> App*/
    {
        let fname = std::path::Path::new(path);
        let file = fs::File::open(&fname).unwrap();
        let mut archive = zip::ZipArchive::new(file).unwrap();

        let manifest: Vec<u8> = zip_readfile(&mut archive, "manifest.bin").unwrap();
        //let manifest_hsm_signature = 

        /*for i in 0..archive.len() {
            let mut file = archive.by_index(i).unwrap();
            println!("Filename: {}", file.name());
            let first_byte = file.bytes().next().unwrap();
        }*/
    }
}
