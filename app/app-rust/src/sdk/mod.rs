#![allow(dead_code)] // XXX

mod allocator;
pub mod crypto;
mod ecall;
pub mod glyphs;
pub mod ux;

use alloc::vec::Vec;
use alloc::{fmt, vec};

#[cfg(target_arch = "riscv32")]
use main;

use self::ecall::*;

#[global_allocator]
static ALLOCATOR: allocator::CAlloc = allocator::CAlloc;

pub enum SdkError {
    KeyGeneration,
    PathDerivation,
    SignatureVerification,
}

impl fmt::Display for SdkError {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        match self {
            SdkError::KeyGeneration => write!(f, "key generation"),
            SdkError::PathDerivation => write!(f, "path derivation"),
            SdkError::SignatureVerification => write!(f, "signature verification"),
        }
    }
}

pub type Result<T> = core::result::Result<T, SdkError>;

pub fn fatal(msg: &str) {
    let buf = msg.as_bytes().to_vec();
    unsafe {
        ecall_fatal(buf.as_ptr());
    }
}

pub fn xrecv(size: usize) -> Vec<u8> {
    let mut buffer = vec![0; size];
    let recv_size = unsafe { ecall_xrecv(buffer.as_mut_ptr(), buffer.len()) };
    buffer[0..recv_size].to_vec()
}

pub fn xsend(buffer: &[u8]) {
    unsafe { ecall_xsend(buffer.as_ptr(), buffer.len() as usize) }
}

#[cfg(target_arch = "riscv32")]
#[panic_handler]
fn my_panic(_info: &core::panic::PanicInfo) -> ! {
    fatal("panic");
    loop {}
}

#[cfg(target_arch = "riscv32")]
#[no_mangle]
pub fn atexit(_f: *const u8) {
    /* required by libcrypto */
    fatal("atexit");
    panic!("atexit called");
}

#[cfg(target_arch = "riscv32")]
#[no_mangle]
pub fn _start(_argc: isize, _argv: *const *const u8) -> isize {
    main();
    0
}
