use alloc::string::{String, ToString};
use alloc::{fmt, format};

use sdk::SdkError;

#[derive(Debug)]
pub struct AppError {
    details: String,
}

impl AppError {
    pub fn new(msg: &str) -> Self {
        Self {
            details: msg.to_string(),
        }
    }
}

impl fmt::Display for AppError {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(f, "{}", self.details)
    }
}

impl From<quick_protobuf::Error> for AppError {
    fn from(err: quick_protobuf::Error) -> Self {
        AppError::new(&err.to_string())
    }
}

impl From<SdkError> for AppError {
    fn from(err: SdkError) -> Self {
        AppError::new(&format!("sdk error: {}", err))
    }
}

pub type Result<T> = core::result::Result<T, AppError>;
