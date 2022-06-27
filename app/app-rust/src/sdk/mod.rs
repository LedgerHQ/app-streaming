#![allow(dead_code)] // XXX

mod allocator;

use alloc::vec;
use alloc::vec::Vec;
use main;

#[global_allocator]
static ALLOCATOR: allocator::CAlloc = allocator::CAlloc;

#[derive(Clone, Copy)]
#[repr(C)]
pub struct CtxSha256 {
    initialized: bool,
    counter: u32,
    blen: usize,
    block: [u8; 64],
    acc: [u8; 8 * 4],
}

#[derive(Clone, Copy)]
#[repr(C)]
union CtxHashGuest {
    sha256: CtxSha256,
}

#[repr(C)]
enum CxHashId {
    HashIdRipemd160,
    HashIdSha3256,
    HashIdSha256,
}

impl Default for CtxSha256 {
    fn default() -> CtxSha256 {
        CtxSha256 {
            initialized: false,
            counter: 0,
            blen: 0,
            block: [0; 64],
            acc: [0; 8 * 4],
        }
    }
}

extern "C" {
    fn ecall_fatal(msg: *const u8);
    fn ecall_xrecv(buffer: *mut u8, size: usize) -> usize;
    fn ecall_xsend(buffer: *const u8, size: usize);
    fn ecall_hash_update(
        hash_id: CxHashId,
        ctx: &mut CtxHashGuest,
        buffer: *const u8,
        size: usize,
    ) -> bool;
    fn ecall_hash_final(hash_id: CxHashId, ctx: &mut CtxHashGuest, buffer: *mut u8) -> bool;
}

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

pub fn sha256_new() -> CtxSha256 {
    CtxSha256::default()
}

pub fn sha256_update(ctx: &mut CtxSha256, buffer: &[u8]) {
    let mut hash_ctx = CtxHashGuest { sha256: *ctx };
    if !unsafe {
        ecall_hash_update(
            CxHashId::HashIdSha256,
            &mut hash_ctx,
            buffer.as_ptr(),
            buffer.len(),
        )
    } {
        fatal("sha256_update");
    }
    *ctx = unsafe { hash_ctx.sha256 };
}

pub fn sha256_final(ctx: &mut CtxSha256) -> [u8; 32] {
    let mut digest = [0u8; 32];
    let mut hash_ctx = CtxHashGuest { sha256: *ctx };
    if !unsafe { ecall_hash_final(CxHashId::HashIdSha256, &mut hash_ctx, digest.as_mut_ptr()) } {
        fatal("sha256_final");
    }
    *ctx = unsafe { hash_ctx.sha256 };

    digest
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
