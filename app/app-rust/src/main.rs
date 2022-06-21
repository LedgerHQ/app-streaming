#![feature(default_alloc_error_handler)]
#![cfg_attr(target_arch = "riscv32", no_main, no_std)]

extern crate alloc;
extern crate quick_protobuf;
extern crate serde;

#[cfg(not(target_arch = "riscv32"))]
extern crate core;

mod error;
mod ledger_swap;
mod message;
mod partner;
mod sdk;
mod swap;
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
            OneOfrequest::init_sell(_init_sell) => {
                OneOfresponse::error(set_error("todo: init_sell"))
            }
            OneOfrequest::swap(swap) => OneOfresponse::swap(handle_swap(&swap)?),
            OneOfrequest::sell(_sell) => OneOfresponse::error(set_error("todo: sell")),
            OneOfrequest::None => OneOfresponse::error(set_error("request unset")),
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
    let request_pb = [b'\n', b'\x00'];
    let response = handle_req(&request_pb);
    assert_eq!(
        response,
        [b'\n', b'\x07', b'\n', b'\x05', b'1', b'.', b'2', b'.', b'3']
    );
}

pub fn main() {
    version::setup_app();

    loop {
        let buffer = sdk::xrecv(512);
        let result = handle_req(&buffer);
        sdk::xsend(&result);
    }
}
