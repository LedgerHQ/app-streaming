#![feature(default_alloc_error_handler)]
#![cfg_attr(target_arch = "riscv32", no_main, no_std)]

extern crate alloc;
extern crate bech32;
extern crate byteorder;
extern crate hex;
extern crate hex_literal;
extern crate primitive_types;
extern crate quick_protobuf;
extern crate serde;

#[cfg(not(target_arch = "riscv32"))]
extern crate core;

mod btc;
mod currency;
mod error;
mod eth;
mod ledger_swap;
mod message;
mod partner;
mod sdk;
mod swap;
mod ui;
mod version;

use alloc::borrow::Cow;
use alloc::string::{String, ToString};
use alloc::vec;
use alloc::vec::Vec;
use quick_protobuf::{BytesReader, BytesWriter, MessageRead, MessageWrite, Writer};

use error::*;
use message::message::mod_Request::OneOfrequest;
use message::message::mod_Response::OneOfresponse;
use message::message::*;
use swap::*;

#[cfg(test)]
use hex_literal::hex;

fn handle_get_version<'a>() -> ResponseGetVersion<'a> {
    ResponseGetVersion {
        version: Cow::Borrowed("1.2.3"),
    }
}

fn set_error(msg: &'_ str) -> ResponseError {
    ResponseError {
        error_msg: Cow::Borrowed(msg),
    }
}

impl From<&'static str> for ResponseError<'_> {
    fn from(msg: &'static str) -> Self {
        ResponseError {
            error_msg: Cow::Borrowed(msg),
        }
    }
}

fn handle_req_(buffer: &[u8]) -> Result<Response> {
    let pb_bytes = buffer.to_vec();
    let mut reader = BytesReader::from_bytes(&pb_bytes);
    let request: Request<'_> = Request::from_reader(&mut reader, &pb_bytes)?;

    let response = Response {
        response: match request.request {
            OneOfrequest::get_version(_) => OneOfresponse::get_version(handle_get_version()),
            OneOfrequest::init_swap(init_swap) => {
                OneOfresponse::init_swap(handle_init_swap(&init_swap))
            }
            OneOfrequest::init_sell(_init_sell) => OneOfresponse::error("todo: init_sell".into()),
            OneOfrequest::swap(swap) => OneOfresponse::swap(handle_swap(&swap)?),
            OneOfrequest::sell(_sell) => OneOfresponse::error("todo: sell".into()),
            OneOfrequest::None => OneOfresponse::error("request unset".into()),
        },
    };

    Ok(response)
}

fn handle_req(buffer: &[u8]) -> Vec<u8> {
    let error_msg: String;

    let response = match handle_req_(buffer) {
        Ok(response) => response,
        Err(error) => {
            error_msg = error.to_string();
            Response {
                response: OneOfresponse::error(set_error(&error_msg)),
            }
        }
    };

    let mut out = vec![0; response.get_size()];
    let mut writer = Writer::new(BytesWriter::new(&mut out));
    response.write_message(&mut writer).unwrap();

    out.to_vec()
}

#[test]
fn test_get_version() {
    let request_pb = hex!("0a00");
    let response = handle_req(&request_pb);
    assert_eq!(response, hex!("0a070a05312e322e33"));
}

pub fn main() {
    version::setup_app();

    loop {
        sdk::ux::ux_idle();

        let buffer = sdk::xrecv(512);

        sdk::ux::app_loading_start("Handling request...\x00");

        let result = handle_req(&buffer);
        sdk::xsend(&result);

        sdk::ux::app_loading_stop();
    }
}
