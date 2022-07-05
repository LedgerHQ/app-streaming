use alloc::string::ToString;
use alloc::{format, vec};
use bech32::ToBase32;
use byteorder::{BigEndian, ByteOrder};
use String;

#[cfg(test)]
use hex_literal::hex;

use currency::*;
use error::{Result, *};
use sdk::crypto::*;

pub struct Btc {}

fn encode_bech32(compressed_pubkey: &[u8]) -> Result<String> {
    let buffer = CtxSha256::new().update(compressed_pubkey).r#final();
    let buffer = CtxRipeMd160::new().update(&buffer).r#final();

    let mut buf32 = vec![bech32::u5::try_from_u8(0).unwrap()];
    buf32.extend(buffer.to_base32());

    bech32::encode("bc", buf32, bech32::Variant::Bech32).map_err(|_| AppError::new("bech32 failed"))
}

impl Currency for Btc {
    fn validate_address(&self, address: &str, path: &[u32]) -> Result<()> {
        let pubkey = EcfpPublicKey::from_path(CxCurve::Secp256k1, path)?;

        let w = &pubkey.as_bytes();
        let mut compressed_pubkey = w[0..33].to_vec();
        compressed_pubkey[0] = if (w[64] as u8) & 1 == 1 {
            b'\x03'
        } else {
            b'\x02'
        };

        let addr = if &address[..3] == "bc1" {
            encode_bech32(&compressed_pubkey)?
        } else if &address[..1] == "1" {
            "TODO legacy".to_string()
        } else {
            "TODO segwit".to_string()
        };

        if address != addr {
            Err(AppError::new("invalid BTC address"))
        } else {
            Ok(())
        }
    }

    fn get_printable_amount(&self, amount: &[u8]) -> Result<String> {
        // BigEndian::read_uint panics when nbytes < 1 or nbytes > 8 or buf.len() < nbytes
        let len = amount.len();
        if !(1..=8).contains(&len) {
            return Err(AppError::new("invalid btc amount"));
        }

        let sat = BigEndian::read_uint(amount, len);
        let btc = (sat as f64) / 100_000_000.;
        let s = format!("BTC {:.15}", btc);

        // remove trailing 0s and . if possible
        Ok(s.trim_end_matches('0').trim_end_matches('.').to_string())
    }

    fn create_tx(&self) {}
}

#[test]
fn test_bech32() {
    assert_eq!(
        encode_bech32(&hex!(
            "0279be667ef9dcbbac55a06295ce870b07029bfcdb2dce28d959f2815b16f81798"
        ))
        .unwrap(),
        "bc1qw508d6qejxtdg4y5r3zarvary0c5xw7kv8f3t4"
    );

    let btc = Btc {};
    btc.validate_address(
        &"bc1qwpgezdcy7g6khsald7cww42lva5g5dmasn6y2z",
        &[0x80000054, 0x80000000, 0x80000000, 1, 0],
    )
    .unwrap();
}

#[test]
fn test_printable_amount() {
    let btc = Btc {};
    assert_eq!(
        btc.get_printable_amount(&hex!("05f5e100")).unwrap(),
        "BTC 1"
    );
    assert_eq!(
        btc.get_printable_amount(&hex!("04d2")).unwrap(),
        "BTC 0.00001234"
    );
    assert_eq!(
        btc.get_printable_amount(&hex!("4c4b40")).unwrap(),
        "BTC 0.05"
    );
}
