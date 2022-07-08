use std::mem;

const MANIFEST_VERSION: u32 = 1;

#[derive(Debug)]
#[repr(C)]
pub struct Manifest {
    manifest_version: u32,
    name: [u8; 32],
    version: [u8; 32],
    app_hash: [u8; 16],
    entrypoint: u32,
    bss: u32,
    code_start: u32,
    code_end: u32,
    stack_start: u32,
    stack_end: u32,
    data_start: u32,
    data_end: u32,
    mt_root_hash: [u8; 32],
    mt_size: u32,
    mt_last_entry: [u8; 8],
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
