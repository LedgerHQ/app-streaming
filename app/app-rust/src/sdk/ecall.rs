use sdk::crypto::{CtxHashGuest, CxCurve, CxHashId, EcfpPrivateKey, EcfpPublicKey};
use sdk::ux::BaglComponent;

extern "C" {
    pub fn ecall_app_loading_start(status: *const u8);
    pub fn ecall_app_loading_stop() -> bool;
    pub fn ecall_bagl_hal_draw_bitmap_within_rect(
        x: i32,
        y: i32,
        width: usize,
        height: usize,
        colors: *const u32,
        bpp: usize,
        bitmap: *const u8,
        bitmap_length_bits: usize,
    );
    pub fn ecall_bagl_draw_with_context(
        component: &BaglComponent,
        context: *const u8,
        context_length: u16,
        context_encoding: u8,
    );
    pub fn ecall_fatal(msg: *const u8);
    pub fn ecall_screen_update();
    pub fn ecall_ux_idle();
    pub fn ecall_xrecv(buffer: *mut u8, size: usize) -> usize;
    pub fn ecall_xsend(buffer: *const u8, size: usize);
    pub fn ecall_wait_button() -> u32;

    pub fn ecall_cx_ecfp_generate_pair(
        curve: CxCurve,
        pubkey: &mut EcfpPublicKey,
        privkey: &mut EcfpPrivateKey,
        keep_privkey: bool,
    ) -> bool;
    pub fn ecall_derive_node_bip32(
        curve: CxCurve,
        path: *const u32,
        path_count: usize,
        privkey_data: *mut u8,
        chain_code: *mut u8,
    ) -> bool;
    pub fn ecall_ecdsa_verify(
        key: &EcfpPublicKey,
        hash: *const u8,
        sig: *const u8,
        sig_len: usize,
    ) -> bool;
    pub fn ecall_get_random_bytes(buffer: *mut u8, size: usize);
    pub fn ecall_hash_update(
        hash_id: CxHashId,
        ctx: &mut CtxHashGuest,
        buffer: *const u8,
        size: usize,
    ) -> bool;
    pub fn ecall_hash_final(hash_id: CxHashId, ctx: &mut CtxHashGuest, buffer: *mut u8) -> bool;
}
