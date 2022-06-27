use error::*;
use message::message::*;
use sdk;

pub fn verify_partner(partner: &Partner<'_>) -> Result<()> {
    let len = partner.name.len();

    let mut ctx = sdk::sha256_new();
    sdk::sha256_update(&mut ctx, &[len as u8; 1]);
    sdk::sha256_update(&mut ctx, partner.name.as_bytes());
    sdk::sha256_update(&mut ctx, &partner.pubkey);
    let digest = sdk::sha256_final(&mut ctx);
    digest.to_vec();

    // XXX: ecdsa verify
    if false {
        return Err(AppError::new("invalid partner"));
    }

    Ok(())
}
