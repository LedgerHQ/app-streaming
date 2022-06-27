#[cfg(test)]
use hex_literal::hex;

use sdk::*;

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
pub struct CtxSha3 {
    initialized: bool,
    counter: u32,
    blen: usize,
    block: [u8; 200],
    acc: [u64; 25],
}

#[derive(Clone, Copy)]
#[repr(C)]
pub struct CtxRipeMd160 {
    initialized: bool,
    counter: u32,
    blen: usize,
    block: [u8; 64],
    acc: [u8; 5 * 4],
}

#[derive(Clone, Copy)]
#[repr(C)]
pub enum CxCurve {
    Secp256k1 = 0x21,
    Secp256r1 = 0x22,
}

#[derive(Clone, Copy)]
#[repr(C)]
pub struct EcfpPublicKey {
    curve: CxCurve,
    w_len: usize,
    w: [u8; 65],
}

#[derive(Clone, Copy)]
#[repr(C)]
pub struct EcfpPrivateKey {
    curve: CxCurve,
    d_len: usize,
    d: [u8; 32],
}

#[derive(Clone, Copy)]
#[repr(C)]
pub union CtxHashGuest {
    ripemd160: CtxRipeMd160,
    sha3: CtxSha3,
    sha256: CtxSha256,
}

#[repr(C)]
pub enum CxHashId {
    HashIdRipeMd160,
    HashIdSha3_256,
    HashIdSha256,
}

impl CtxSha256 {
    pub fn new() -> Self {
        CtxSha256 {
            initialized: false,
            counter: 0,
            blen: 0,
            block: [0; 64],
            acc: [0; 8 * 4],
        }
    }

    pub fn update(&mut self, buffer: &[u8]) -> Self {
        let mut hash_ctx = CtxHashGuest { sha256: *self };
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
        *self = unsafe { hash_ctx.sha256 };
        *self
    }

    pub fn r#final(&mut self) -> [u8; 32] {
        let mut digest = [0u8; 32];
        let mut hash_ctx = CtxHashGuest { sha256: *self };
        if !unsafe { ecall_hash_final(CxHashId::HashIdSha256, &mut hash_ctx, digest.as_mut_ptr()) }
        {
            fatal("sha256_final");
        }
        *self = unsafe { hash_ctx.sha256 };

        digest
    }
}

impl CtxRipeMd160 {
    pub fn new() -> Self {
        CtxRipeMd160 {
            initialized: false,
            counter: 0,
            blen: 0,
            block: [0; 64],
            acc: [0; 5 * 4],
        }
    }

    pub fn update(&mut self, buffer: &[u8]) -> Self {
        let mut hash_ctx = CtxHashGuest { ripemd160: *self };
        if !unsafe {
            ecall_hash_update(
                CxHashId::HashIdRipeMd160,
                &mut hash_ctx,
                buffer.as_ptr(),
                buffer.len(),
            )
        } {
            fatal("ripemd160_update");
        }
        *self = unsafe { hash_ctx.ripemd160 };
        *self
    }

    pub fn r#final(&mut self) -> [u8; 20] {
        let mut digest = [0u8; 20];
        let mut hash_ctx = CtxHashGuest { ripemd160: *self };
        if !unsafe {
            ecall_hash_final(
                CxHashId::HashIdRipeMd160,
                &mut hash_ctx,
                digest.as_mut_ptr(),
            )
        } {
            fatal("ripemd160_final");
        }
        *self = unsafe { hash_ctx.ripemd160 };

        digest
    }
}

impl CtxSha3 {
    pub fn new() -> Self {
        CtxSha3 {
            initialized: false,
            counter: 0,
            blen: 0,
            block: [0; 200],
            acc: [0; 25],
        }
    }

    pub fn update(&mut self, buffer: &[u8]) -> Self {
        let mut hash_ctx = CtxHashGuest { sha3: *self };
        if !unsafe {
            ecall_hash_update(
                CxHashId::HashIdSha3_256,
                &mut hash_ctx,
                buffer.as_ptr(),
                buffer.len(),
            )
        } {
            fatal("sha3_update");
        }
        *self = unsafe { hash_ctx.sha3 };
        *self
    }

    pub fn r#final(&mut self) -> [u8; 32] {
        let mut digest = [0u8; 32];
        let mut hash_ctx = CtxHashGuest { sha3: *self };
        if !unsafe {
            ecall_hash_final(CxHashId::HashIdSha3_256, &mut hash_ctx, digest.as_mut_ptr())
        } {
            fatal("sha3_final");
        }
        *self = unsafe { hash_ctx.sha3 };

        digest
    }
}

pub fn derive_node_bip32(
    curve: CxCurve,
    path: &[u32],
    privkey_data: Option<&mut [u8]>,
    chain_code: Option<&mut [u8]>,
) -> Result<()> {
    let privkey_data = if let Some(p) = privkey_data {
        p.as_mut_ptr()
    } else {
        core::ptr::null_mut()
    };
    let chain_code = if let Some(p) = chain_code {
        p.as_mut_ptr()
    } else {
        core::ptr::null_mut()
    };
    if !unsafe {
        ecall_derive_node_bip32(curve, path.as_ptr(), path.len(), privkey_data, chain_code)
    } {
        Err(SdkError::PathDerivation)
    } else {
        Ok(())
    }
}

pub fn ecfp_generate_keypair(
    curve: CxCurve,
    pubkey: &mut EcfpPublicKey,
    privkey: &mut EcfpPrivateKey,
    keep_privkey: bool,
) -> Result<()> {
    if !unsafe { ecall_cx_ecfp_generate_pair(curve, pubkey, privkey, keep_privkey) } {
        Err(SdkError::KeyGeneration)
    } else {
        Ok(())
    }
}

pub fn get_random_bytes(buffer: &mut [u8]) {
    unsafe {
        ecall_get_random_bytes(buffer.as_mut_ptr(), buffer.len());
    }
}

impl EcfpPublicKey {
    pub fn new(curve: CxCurve, bytes: &[u8; 65]) -> Self {
        Self {
            curve,
            w_len: 65,
            w: *bytes,
        }
    }

    pub fn as_bytes(&self) -> &[u8; 65] {
        &self.w
    }

    pub fn verify(&self, hash: &[u8; 32], sig: &[u8]) -> Result<()> {
        if !unsafe { ecall_ecdsa_verify(self, hash.as_ptr(), sig.as_ptr(), sig.len()) } {
            Err(SdkError::SignatureVerification)
        } else {
            Ok(())
        }
    }

    pub fn from_path(curve: CxCurve, path: &[u32]) -> Result<EcfpPublicKey> {
        let privkey_data = &mut [0u8; 32];
        derive_node_bip32(curve, path, Some(privkey_data), None)?;
        let mut privkey = EcfpPrivateKey::new(curve, privkey_data);
        let mut pubkey = EcfpPublicKey::new(curve, &[0u8; 65]);
        ecfp_generate_keypair(curve, &mut pubkey, &mut privkey, true)?;
        Ok(pubkey)
    }
}

impl EcfpPrivateKey {
    pub fn new(curve: CxCurve, bytes: &[u8; 32]) -> Self {
        Self {
            curve,
            d_len: 32,
            d: *bytes,
        }
    }
}

#[test]
fn test_ripemd160() {
    let buffer = hex!("616263");

    let digest = CtxRipeMd160::new().update(&buffer).r#final();

    assert_eq!(digest, hex!("8eb208f7e05d987a9b044a8e98c6b087f15a0bfc"));
}

#[test]
fn test_sha256() {
    let buffer = hex!("616263");

    let digest = CtxSha256::new().update(&buffer).r#final();

    assert_eq!(
        digest,
        hex!("ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad")
    );
}

#[test]
fn test_sha3() {
    assert_eq!(
        CtxSha3::new().update(&hex!("")).r#final(),
        hex!("C5D2460186F7233C927E7DB2DCC703C0E500B653CA82273B7BFAD8045D85A470")
    );
    assert_eq!(
        CtxSha3::new().update(&hex!("41FB")).r#final(),
        hex!("A8EACEDA4D47B3281A795AD9E1EA2122B407BAF9AABCB9E18B5717B7873537D2")
    );
    assert_eq!(
        CtxSha3::new().update(b"Hello").r#final(),
        hex!("06b3dfaec148fb1bb2b066f10ec285e7c9bf402ab32aa78a5d38e34566810cd2")
    );
    let data = &hex!("836b35a026743e823a90a0ee3b91bf615c6a757e2b60b9e1dc1826fd0dd16106f7bc1e8179f665015f43c6c81f39062fc2086ed849625c06e04697698b21855e");
    assert_eq!(
        CtxSha3::new().update(data).r#final(),
        hex!("72f15d6555488541650ce62c0bed7abd61247635c1973eb38474a2516ed1d884")
    );
}
