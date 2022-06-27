use alloc::boxed::Box;
use String;

use btc::*;
use error::*;
use eth::*;

pub trait Currency {
    fn validate_address(&self, address: &str, path: &[u32]) -> Result<()>;
    fn get_printable_amount(&self, amount: &[u8]) -> Result<String>;
    fn create_tx(&self);
}

pub fn get_currency(name: &str) -> Result<Box<dyn Currency>> {
    match name {
        "BTC" => Ok(Box::new(Btc {})),
        "ETH" => Ok(Box::new(Eth {})),
        _ => Err(AppError::new("invalid currency")),
    }
}
