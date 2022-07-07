use std::convert::TryInto;
use std::fs;
use std::io::{Read, Seek};

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
    manifest_device_signature: Vec<u8>,
}

fn zip_readfile<'a, R>(archive: &'a mut zip::ZipArchive<R>, name: &str) -> Option<Vec<u8>>
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
        assert!(data.len() % PAGE_SIZE == 0);

        data.chunks(PAGE_SIZE)
            .map(|x| x.try_into().unwrap())
            .collect()
    }

    pub fn from_zip(path: &str) -> App {
        let fname = std::path::Path::new(path);
        let file = fs::File::open(&fname).unwrap();
        let mut archive = zip::ZipArchive::new(file).unwrap();

        let mut app = App::default();

        app.manifest = zip_readfile(&mut archive, "manifest.bin").unwrap();
        app.manifest_hsm_signature = zip_readfile(&mut archive, "manifest.hsm.sig").unwrap();
        app.code_pages = App::pages_to_list(&zip_readfile(&mut archive, "code.bin").unwrap());

        app
    }
}
