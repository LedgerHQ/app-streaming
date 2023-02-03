use alloc::borrow::Cow;
use core::convert::TryInto;
use quick_protobuf::{BytesReader, MessageRead};

use currency::*;
use error::*;
use ledger_swap::ledger_swap::*;
use message::message::*;
use partner::*;
use exapp_sdk::crypto::{get_random_bytes, CtxSha256, CxCurve, EcfpPublicKey};
use exapp_sdk::ux::app_loading_start;
use ui::sign_tx_validation;

// mutable static is unsafe (because of threads)
static mut DEVICE_ID: [u8; 10] = [0u8; 10];

fn valid_id() -> Result<()> {
    if unsafe { DEVICE_ID.iter() }.all(|c| *c == b'\x00') {
        return Err(AppError::new("device id unset"));
    }
    Ok(())
}

pub fn handle_init_swap<'a>(_swap: &RequestInitSwap) -> ResponseInitSwap<'a> {
    get_random_bytes(unsafe { &mut DEVICE_ID });
    ResponseInitSwap {
        device_id: Cow::Owned(unsafe { DEVICE_ID.to_vec() }),
    }
}

fn verify_tx_signature(raw_pubkey: &[u8; 65], tx: &[u8], signature: &[u8]) -> Result<()> {
    let digest = CtxSha256::new().update(tx).r#final();

    let pubkey = EcfpPublicKey::new(CxCurve::Secp256k1, raw_pubkey);
    pubkey.verify(&digest, signature)?;
    Ok(())
}

pub fn handle_swap<'a>(swap: &RequestSwap<'_>) -> Result<ResponseSwap<'a>> {
    let partner = swap
        .partner
        .as_ref()
        .ok_or_else(|| AppError::new("partner unset"))?;
    verify_partner(partner)?;

    let pb_bytes = &swap.pb_tx;
    let mut reader = BytesReader::from_bytes(pb_bytes);
    let request = NewTransactionResponse::from_reader(&mut reader, pb_bytes)?;

    valid_id()?;

    let raw_pubkey: &[u8; 65] = &partner
        .pubkey
        .as_ref()
        .try_into()
        .map_err(|_| AppError::new("invalid partner pubkey size"))?;
    verify_tx_signature(raw_pubkey, &swap.pb_tx, &swap.signature)?;

    let from = get_currency(&request.currency_from)?;
    let to = get_currency(&request.currency_to)?;

    from.validate_address(&request.refund_address, &swap.refund_path)?;
    to.validate_address(&request.payout_address, &swap.payout_path)?;

    let send_amount = from.get_printable_amount(&request.amount_to_provider)?;
    let recv_amount = to.get_printable_amount(&request.amount_to_wallet)?;
    let fees = from.get_printable_amount(&swap.fee)?;

    if sign_tx_validation(&send_amount, &recv_amount, &fees) {
        app_loading_start("Signing transaction...\x00");
    }

    Ok(ResponseSwap {
        approved: false,
        tx: Cow::Borrowed(&[0u8; 1]),
    })
}
