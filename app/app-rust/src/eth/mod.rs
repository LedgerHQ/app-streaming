use alloc::string::ToString;
use String;

use currency::*;
use error::*;
use sdk::crypto::*;

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

    fn get_printable_amount(&self, _amount: &[u8]) -> Result<String> {
        Ok("ETH ?".to_string())
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
