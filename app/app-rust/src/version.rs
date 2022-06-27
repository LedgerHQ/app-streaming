#[used]
#[no_mangle]
#[link_section = ".app_name"]
pub static APP_NAME: [u8; 32] = *b"Rust Swap\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00";

#[used]
#[no_mangle]
#[link_section = ".app_version"]
pub static APP_VERSION: [u8; 16] = *b"0.1\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00";

pub fn setup_app() {
    /* Force .app_name and .app_version to be embedded in the binary.
     * TODO: find a better solution */
    unsafe {
        core::ptr::read_volatile(&APP_NAME);
        core::ptr::read_volatile(&APP_VERSION);
    }
}
