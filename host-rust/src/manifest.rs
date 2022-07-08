use std::mem;

const MANIFEST_VERSION: u32 = 1;

#[derive(Debug)]
#[repr(C)]
pub struct Manifest {
    pub manifest_version: u32,
    pub name: [u8; 32],
    pub version: [u8; 16],
    pub app_hash: [u8; 32],
    pub entrypoint: u32,
    pub bss: u32,
    pub code_start: u32,
    pub code_end: u32,
    pub stack_start: u32,
    pub stack_end: u32,
    pub data_start: u32,
    pub data_end: u32,
    pub mt_root_hash: [u8; 32],
    pub mt_size: u32,
    pub mt_last_entry: [u8; 8],
}

const MANIFEST_SIZE: usize = mem::size_of::<Manifest>();

unsafe fn any_as_u8_slice<T: Sized>(p: &T) -> &[u8] {
    ::std::slice::from_raw_parts((p as *const T) as *const u8, ::std::mem::size_of::<T>())
}

impl Manifest {
    pub fn from_bytes(data: &Vec<u8>) -> Self {
        assert_eq!(data.len(), MANIFEST_SIZE);
        let data = data.as_ptr() as *const [u8; MANIFEST_SIZE];
        unsafe { std::mem::transmute(*data) }
    }

    pub fn to_bytes(&self) -> Vec<u8> {
        unsafe { any_as_u8_slice(&self).to_vec() }
    }
}
