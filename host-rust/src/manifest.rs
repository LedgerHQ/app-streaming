use std::mem;

use serialization::{Deserialize, Serialize};

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

pub const MANIFEST_SIZE: usize = mem::size_of::<Manifest>();

impl Deserialize for Manifest {}
impl Serialize for Manifest {}
