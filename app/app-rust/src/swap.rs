use alloc::borrow::Cow;
use quick_protobuf::{BytesReader, MessageRead};

use error::*;
use ledger_swap::ledger_swap::*;
use message::message::*;
use partner::*;

pub fn handle_init_swap<'a>(_swap: &RequestInitSwap) -> ResponseInitSwap<'a> {
    let device_id = [b'Q'; 10].to_vec();
    ResponseInitSwap {
        device_id: Cow::Owned(device_id),
    }
}

fn valid_id(id: &str) -> Result<()> {
    if id.chars().all(|c| c == '\x00') {
        return Err(AppError::new("device id unset"));
    }
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
    let request: NewTransactionResponse<'_> =
        NewTransactionResponse::from_reader(&mut reader, pb_bytes)?;

    //valid_id(&ctx.device_transaction_id)?;

    Ok(ResponseSwap {
        approved: false,
        tx: Cow::Borrowed(&[0u8; 1]),
    })
}
