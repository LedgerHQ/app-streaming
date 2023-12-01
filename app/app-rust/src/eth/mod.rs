use alloc::format;
use alloc::string::ToString;
use primitive_types::U256;
use String;

#[cfg(test)]
use hex_literal::hex;

use currency::*;
use error::*;
use exapp_sdk::crypto::*;

pub struct Eth {}

impl Currency for Eth {
    fn validate_address(&self, address: &str, path: &[u32]) -> Result<()> {
        let pubkey = EcfpPublicKey::from_path(CxCurve::Secp256k1, path)?;

        let digest = CtxSha3::new().update(&pubkey.as_bytes()[1..]).r#final();
        let address = if let Some(address) = address.strip_prefix("0x") {
            address
        } else {
            address
        };

        if address.to_lowercase() != hex::encode(&digest[12..]) {
            Err(AppError::new("invalid ETH address"))
        } else {
            Ok(())
        }
    }

    fn get_printable_amount(&self, amount: &[u8]) -> Result<String> {
        let wei = U256::from_big_endian(amount);
        let wei = format!("{:018}", wei);
        let len = wei.len();
        let eth = if len == 18 {
            format!("ETH 0.{}", wei)
        } else {
            format!("ETH {}.{}", &wei[..len - 18], &wei[len - 18..])
        };

        Ok(eth.trim_end_matches('0').trim_end_matches('.').to_string())
    }

    fn create_tx(&self) {}
}

#[test]
fn test_eth_address() {
    let eth = Eth {};
    eth.validate_address(
        &"0xDad77910DbDFdE764fC21FCD4E74D71bBACA6D8D",
        &[0x8000002c, 0x8000003c, 0x80000000, 0, 0],
    )
    .unwrap();
}

#[test]
fn test_eth_amount() {
    let eth = Eth {};
    assert_eq!(
        eth.get_printable_amount(&hex!("01")).unwrap(),
        "ETH 0.000000000000000001"
    );
    assert_eq!(
        eth.get_printable_amount(&hex!("9983441cbea000")).unwrap(),
        "ETH 0.04321"
    );
    assert_eq!(
        eth.get_printable_amount(&hex!("0de0b6b3a7640000")).unwrap(),
        "ETH 1"
    );
    assert_eq!(
        eth.get_printable_amount(&hex!("06b14bd1e6eea00000"))
            .unwrap(),
        "ETH 123.456"
    );
}
