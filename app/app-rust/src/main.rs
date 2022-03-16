#![no_std]
#![no_main]

//use core::arch::asm;

#[used]
#[no_mangle]
#[link_section = ".app_name"]
pub static APP_NAME: [u8; 32] = *b"Rust App\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00";

#[used]
#[no_mangle]
#[link_section = ".app_version"]
pub static APP_VERSION: [u8; 16] = *b"0.1\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00";

#[panic_handler]
fn my_panic(_info: &core::panic::PanicInfo) -> ! {
    loop {}
}

extern "C" {
    pub fn ecall_fatal(msg: *const str);
    //pub fn ecall_fatal(msg: &[char; 32]);
}

#[no_mangle]
pub fn atexit(f: *const u8) {
    /* required by libcrypto */
    panic!("atexit called");
}

#[no_mangle]
pub fn _start(_argc: isize, _argv: *const *const u8) -> isize {
    /* Force .app_name and .app_version to be embedded in the binary.
     * TODO: find a better solution */
    unsafe {
        core::ptr::read_volatile(&APP_NAME);
        core::ptr::read_volatile(&APP_VERSION);
    }
    unsafe {
        ecall_fatal("hello world\n\x00");
    }
    0
}
