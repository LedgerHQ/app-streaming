// Automatically generated rust module for 'message.proto' file

#![allow(non_snake_case)]
#![allow(non_upper_case_globals)]
#![allow(non_camel_case_types)]
#![allow(unused_imports)]
#![allow(unknown_lints)]
#![allow(clippy::all)]
#![cfg_attr(rustfmt, rustfmt_skip)]


use alloc::vec::Vec;
use alloc::borrow::Cow;
use quick_protobuf::{MessageRead, MessageWrite, BytesReader, Writer, WriterBackend, Result};
use quick_protobuf::sizeofs::*;
use super::*;

#[derive(Debug, Default, PartialEq, Clone)]
pub struct RequestGetVersion { }

impl<'a> MessageRead<'a> for RequestGetVersion {
    fn from_reader(r: &mut BytesReader, _: &[u8]) -> Result<Self> {
        r.read_to_end();
        Ok(Self::default())
    }
}

impl MessageWrite for RequestGetVersion { }

#[derive(Debug, Default, PartialEq, Clone)]
pub struct ResponseGetVersion<'a> {
    pub version: Cow<'a, str>,
}

impl<'a> MessageRead<'a> for ResponseGetVersion<'a> {
    fn from_reader(r: &mut BytesReader, bytes: &'a [u8]) -> Result<Self> {
        let mut msg = Self::default();
        while !r.is_eof() {
            match r.next_tag(bytes) {
                Ok(10) => msg.version = r.read_string(bytes).map(Cow::Borrowed)?,
                Ok(t) => { r.read_unknown(bytes, t)?; }
                Err(e) => return Err(e),
            }
        }
        Ok(msg)
    }
}

impl<'a> MessageWrite for ResponseGetVersion<'a> {
    fn get_size(&self) -> usize {
        0
        + if self.version == "" { 0 } else { 1 + sizeof_len((&self.version).len()) }
    }

    fn write_message<W: WriterBackend>(&self, w: &mut Writer<W>) -> Result<()> {
        if self.version != "" { w.write_with_tag(10, |w| w.write_string(&**&self.version))?; }
        Ok(())
    }
}

#[derive(Debug, Default, PartialEq, Clone)]
pub struct RequestInitSwap { }

impl<'a> MessageRead<'a> for RequestInitSwap {
    fn from_reader(r: &mut BytesReader, _: &[u8]) -> Result<Self> {
        r.read_to_end();
        Ok(Self::default())
    }
}

impl MessageWrite for RequestInitSwap { }

#[derive(Debug, Default, PartialEq, Clone)]
pub struct ResponseInitSwap<'a> {
    pub device_id: Cow<'a, [u8]>,
}

impl<'a> MessageRead<'a> for ResponseInitSwap<'a> {
    fn from_reader(r: &mut BytesReader, bytes: &'a [u8]) -> Result<Self> {
        let mut msg = Self::default();
        while !r.is_eof() {
            match r.next_tag(bytes) {
                Ok(10) => msg.device_id = r.read_bytes(bytes).map(Cow::Borrowed)?,
                Ok(t) => { r.read_unknown(bytes, t)?; }
                Err(e) => return Err(e),
            }
        }
        Ok(msg)
    }
}

impl<'a> MessageWrite for ResponseInitSwap<'a> {
    fn get_size(&self) -> usize {
        0
        + if self.device_id == Cow::Borrowed(b"") { 0 } else { 1 + sizeof_len((&self.device_id).len()) }
    }

    fn write_message<W: WriterBackend>(&self, w: &mut Writer<W>) -> Result<()> {
        if self.device_id != Cow::Borrowed(b"") { w.write_with_tag(10, |w| w.write_bytes(&**&self.device_id))?; }
        Ok(())
    }
}

#[derive(Debug, Default, PartialEq, Clone)]
pub struct RequestInitSell { }

impl<'a> MessageRead<'a> for RequestInitSell {
    fn from_reader(r: &mut BytesReader, _: &[u8]) -> Result<Self> {
        r.read_to_end();
        Ok(Self::default())
    }
}

impl MessageWrite for RequestInitSell { }

#[derive(Debug, Default, PartialEq, Clone)]
pub struct ResponseInitSell<'a> {
    pub device_id: Cow<'a, [u8]>,
}

impl<'a> MessageRead<'a> for ResponseInitSell<'a> {
    fn from_reader(r: &mut BytesReader, bytes: &'a [u8]) -> Result<Self> {
        let mut msg = Self::default();
        while !r.is_eof() {
            match r.next_tag(bytes) {
                Ok(10) => msg.device_id = r.read_bytes(bytes).map(Cow::Borrowed)?,
                Ok(t) => { r.read_unknown(bytes, t)?; }
                Err(e) => return Err(e),
            }
        }
        Ok(msg)
    }
}

impl<'a> MessageWrite for ResponseInitSell<'a> {
    fn get_size(&self) -> usize {
        0
        + if self.device_id == Cow::Borrowed(b"") { 0 } else { 1 + sizeof_len((&self.device_id).len()) }
    }

    fn write_message<W: WriterBackend>(&self, w: &mut Writer<W>) -> Result<()> {
        if self.device_id != Cow::Borrowed(b"") { w.write_with_tag(10, |w| w.write_bytes(&**&self.device_id))?; }
        Ok(())
    }
}

#[derive(Debug, Default, PartialEq, Clone)]
pub struct Partner<'a> {
    pub name: Cow<'a, str>,
    pub pubkey: Cow<'a, [u8]>,
    pub signature: Cow<'a, [u8]>,
}

impl<'a> MessageRead<'a> for Partner<'a> {
    fn from_reader(r: &mut BytesReader, bytes: &'a [u8]) -> Result<Self> {
        let mut msg = Self::default();
        while !r.is_eof() {
            match r.next_tag(bytes) {
                Ok(10) => msg.name = r.read_string(bytes).map(Cow::Borrowed)?,
                Ok(18) => msg.pubkey = r.read_bytes(bytes).map(Cow::Borrowed)?,
                Ok(26) => msg.signature = r.read_bytes(bytes).map(Cow::Borrowed)?,
                Ok(t) => { r.read_unknown(bytes, t)?; }
                Err(e) => return Err(e),
            }
        }
        Ok(msg)
    }
}

impl<'a> MessageWrite for Partner<'a> {
    fn get_size(&self) -> usize {
        0
        + if self.name == "" { 0 } else { 1 + sizeof_len((&self.name).len()) }
        + if self.pubkey == Cow::Borrowed(b"") { 0 } else { 1 + sizeof_len((&self.pubkey).len()) }
        + if self.signature == Cow::Borrowed(b"") { 0 } else { 1 + sizeof_len((&self.signature).len()) }
    }

    fn write_message<W: WriterBackend>(&self, w: &mut Writer<W>) -> Result<()> {
        if self.name != "" { w.write_with_tag(10, |w| w.write_string(&**&self.name))?; }
        if self.pubkey != Cow::Borrowed(b"") { w.write_with_tag(18, |w| w.write_bytes(&**&self.pubkey))?; }
        if self.signature != Cow::Borrowed(b"") { w.write_with_tag(26, |w| w.write_bytes(&**&self.signature))?; }
        Ok(())
    }
}

#[derive(Debug, Default, PartialEq, Clone)]
pub struct RequestSwap<'a> {
    pub partner: Option<Partner<'a>>,
    pub pb_tx: Cow<'a, [u8]>,
    pub signature: Cow<'a, [u8]>,
    pub fee: Cow<'a, [u8]>,
    pub payout_path: Vec<u32>,
    pub refund_path: Vec<u32>,
    pub payout_addr_params: Cow<'a, [u8]>,
    pub refund_addr_params: Cow<'a, [u8]>,
}

impl<'a> MessageRead<'a> for RequestSwap<'a> {
    fn from_reader(r: &mut BytesReader, bytes: &'a [u8]) -> Result<Self> {
        let mut msg = Self::default();
        while !r.is_eof() {
            match r.next_tag(bytes) {
                Ok(10) => msg.partner = Some(r.read_message::<Partner>(bytes)?),
                Ok(18) => msg.pb_tx = r.read_bytes(bytes).map(Cow::Borrowed)?,
                Ok(26) => msg.signature = r.read_bytes(bytes).map(Cow::Borrowed)?,
                Ok(34) => msg.fee = r.read_bytes(bytes).map(Cow::Borrowed)?,
                Ok(42) => msg.payout_path = r.read_packed(bytes, |r, bytes| Ok(r.read_uint32(bytes)?))?,
                Ok(50) => msg.refund_path = r.read_packed(bytes, |r, bytes| Ok(r.read_uint32(bytes)?))?,
                Ok(58) => msg.payout_addr_params = r.read_bytes(bytes).map(Cow::Borrowed)?,
                Ok(66) => msg.refund_addr_params = r.read_bytes(bytes).map(Cow::Borrowed)?,
                Ok(t) => { r.read_unknown(bytes, t)?; }
                Err(e) => return Err(e),
            }
        }
        Ok(msg)
    }
}

impl<'a> MessageWrite for RequestSwap<'a> {
    fn get_size(&self) -> usize {
        0
        + self.partner.as_ref().map_or(0, |m| 1 + sizeof_len((m).get_size()))
        + if self.pb_tx == Cow::Borrowed(b"") { 0 } else { 1 + sizeof_len((&self.pb_tx).len()) }
        + if self.signature == Cow::Borrowed(b"") { 0 } else { 1 + sizeof_len((&self.signature).len()) }
        + if self.fee == Cow::Borrowed(b"") { 0 } else { 1 + sizeof_len((&self.fee).len()) }
        + if self.payout_path.is_empty() { 0 } else { 1 + sizeof_len(self.payout_path.iter().map(|s| sizeof_varint(*(s) as u64)).sum::<usize>()) }
        + if self.refund_path.is_empty() { 0 } else { 1 + sizeof_len(self.refund_path.iter().map(|s| sizeof_varint(*(s) as u64)).sum::<usize>()) }
        + if self.payout_addr_params == Cow::Borrowed(b"") { 0 } else { 1 + sizeof_len((&self.payout_addr_params).len()) }
        + if self.refund_addr_params == Cow::Borrowed(b"") { 0 } else { 1 + sizeof_len((&self.refund_addr_params).len()) }
    }

    fn write_message<W: WriterBackend>(&self, w: &mut Writer<W>) -> Result<()> {
        if let Some(ref s) = self.partner { w.write_with_tag(10, |w| w.write_message(s))?; }
        if self.pb_tx != Cow::Borrowed(b"") { w.write_with_tag(18, |w| w.write_bytes(&**&self.pb_tx))?; }
        if self.signature != Cow::Borrowed(b"") { w.write_with_tag(26, |w| w.write_bytes(&**&self.signature))?; }
        if self.fee != Cow::Borrowed(b"") { w.write_with_tag(34, |w| w.write_bytes(&**&self.fee))?; }
        w.write_packed_with_tag(42, &self.payout_path, |w, m| w.write_uint32(*m), &|m| sizeof_varint(*(m) as u64))?;
        w.write_packed_with_tag(50, &self.refund_path, |w, m| w.write_uint32(*m), &|m| sizeof_varint(*(m) as u64))?;
        if self.payout_addr_params != Cow::Borrowed(b"") { w.write_with_tag(58, |w| w.write_bytes(&**&self.payout_addr_params))?; }
        if self.refund_addr_params != Cow::Borrowed(b"") { w.write_with_tag(66, |w| w.write_bytes(&**&self.refund_addr_params))?; }
        Ok(())
    }
}

#[derive(Debug, Default, PartialEq, Clone)]
pub struct ResponseSwap<'a> {
    pub approved: bool,
    pub tx: Cow<'a, [u8]>,
}

impl<'a> MessageRead<'a> for ResponseSwap<'a> {
    fn from_reader(r: &mut BytesReader, bytes: &'a [u8]) -> Result<Self> {
        let mut msg = Self::default();
        while !r.is_eof() {
            match r.next_tag(bytes) {
                Ok(8) => msg.approved = r.read_bool(bytes)?,
                Ok(18) => msg.tx = r.read_bytes(bytes).map(Cow::Borrowed)?,
                Ok(t) => { r.read_unknown(bytes, t)?; }
                Err(e) => return Err(e),
            }
        }
        Ok(msg)
    }
}

impl<'a> MessageWrite for ResponseSwap<'a> {
    fn get_size(&self) -> usize {
        0
        + if self.approved == false { 0 } else { 1 + sizeof_varint(*(&self.approved) as u64) }
        + if self.tx == Cow::Borrowed(b"") { 0 } else { 1 + sizeof_len((&self.tx).len()) }
    }

    fn write_message<W: WriterBackend>(&self, w: &mut Writer<W>) -> Result<()> {
        if self.approved != false { w.write_with_tag(8, |w| w.write_bool(*&self.approved))?; }
        if self.tx != Cow::Borrowed(b"") { w.write_with_tag(18, |w| w.write_bytes(&**&self.tx))?; }
        Ok(())
    }
}

#[derive(Debug, Default, PartialEq, Clone)]
pub struct RequestSell<'a> {
    pub partner: Option<Partner<'a>>,
    pub b64_tx: Cow<'a, [u8]>,
    pub signature: Cow<'a, [u8]>,
    pub fee: Cow<'a, [u8]>,
}

impl<'a> MessageRead<'a> for RequestSell<'a> {
    fn from_reader(r: &mut BytesReader, bytes: &'a [u8]) -> Result<Self> {
        let mut msg = Self::default();
        while !r.is_eof() {
            match r.next_tag(bytes) {
                Ok(10) => msg.partner = Some(r.read_message::<Partner>(bytes)?),
                Ok(18) => msg.b64_tx = r.read_bytes(bytes).map(Cow::Borrowed)?,
                Ok(26) => msg.signature = r.read_bytes(bytes).map(Cow::Borrowed)?,
                Ok(34) => msg.fee = r.read_bytes(bytes).map(Cow::Borrowed)?,
                Ok(t) => { r.read_unknown(bytes, t)?; }
                Err(e) => return Err(e),
            }
        }
        Ok(msg)
    }
}

impl<'a> MessageWrite for RequestSell<'a> {
    fn get_size(&self) -> usize {
        0
        + self.partner.as_ref().map_or(0, |m| 1 + sizeof_len((m).get_size()))
        + if self.b64_tx == Cow::Borrowed(b"") { 0 } else { 1 + sizeof_len((&self.b64_tx).len()) }
        + if self.signature == Cow::Borrowed(b"") { 0 } else { 1 + sizeof_len((&self.signature).len()) }
        + if self.fee == Cow::Borrowed(b"") { 0 } else { 1 + sizeof_len((&self.fee).len()) }
    }

    fn write_message<W: WriterBackend>(&self, w: &mut Writer<W>) -> Result<()> {
        if let Some(ref s) = self.partner { w.write_with_tag(10, |w| w.write_message(s))?; }
        if self.b64_tx != Cow::Borrowed(b"") { w.write_with_tag(18, |w| w.write_bytes(&**&self.b64_tx))?; }
        if self.signature != Cow::Borrowed(b"") { w.write_with_tag(26, |w| w.write_bytes(&**&self.signature))?; }
        if self.fee != Cow::Borrowed(b"") { w.write_with_tag(34, |w| w.write_bytes(&**&self.fee))?; }
        Ok(())
    }
}

#[derive(Debug, Default, PartialEq, Clone)]
pub struct ResponseSell<'a> {
    pub approved: bool,
    pub signature: Cow<'a, [u8]>,
}

impl<'a> MessageRead<'a> for ResponseSell<'a> {
    fn from_reader(r: &mut BytesReader, bytes: &'a [u8]) -> Result<Self> {
        let mut msg = Self::default();
        while !r.is_eof() {
            match r.next_tag(bytes) {
                Ok(8) => msg.approved = r.read_bool(bytes)?,
                Ok(18) => msg.signature = r.read_bytes(bytes).map(Cow::Borrowed)?,
                Ok(t) => { r.read_unknown(bytes, t)?; }
                Err(e) => return Err(e),
            }
        }
        Ok(msg)
    }
}

impl<'a> MessageWrite for ResponseSell<'a> {
    fn get_size(&self) -> usize {
        0
        + if self.approved == false { 0 } else { 1 + sizeof_varint(*(&self.approved) as u64) }
        + if self.signature == Cow::Borrowed(b"") { 0 } else { 1 + sizeof_len((&self.signature).len()) }
    }

    fn write_message<W: WriterBackend>(&self, w: &mut Writer<W>) -> Result<()> {
        if self.approved != false { w.write_with_tag(8, |w| w.write_bool(*&self.approved))?; }
        if self.signature != Cow::Borrowed(b"") { w.write_with_tag(18, |w| w.write_bytes(&**&self.signature))?; }
        Ok(())
    }
}

#[derive(Debug, Default, PartialEq, Clone)]
pub struct ResponseError<'a> {
    pub error_msg: Cow<'a, str>,
}

impl<'a> MessageRead<'a> for ResponseError<'a> {
    fn from_reader(r: &mut BytesReader, bytes: &'a [u8]) -> Result<Self> {
        let mut msg = Self::default();
        while !r.is_eof() {
            match r.next_tag(bytes) {
                Ok(10) => msg.error_msg = r.read_string(bytes).map(Cow::Borrowed)?,
                Ok(t) => { r.read_unknown(bytes, t)?; }
                Err(e) => return Err(e),
            }
        }
        Ok(msg)
    }
}

impl<'a> MessageWrite for ResponseError<'a> {
    fn get_size(&self) -> usize {
        0
        + if self.error_msg == "" { 0 } else { 1 + sizeof_len((&self.error_msg).len()) }
    }

    fn write_message<W: WriterBackend>(&self, w: &mut Writer<W>) -> Result<()> {
        if self.error_msg != "" { w.write_with_tag(10, |w| w.write_string(&**&self.error_msg))?; }
        Ok(())
    }
}

#[derive(Debug, Default, PartialEq, Clone)]
pub struct Request<'a> {
    pub request: mod_Request::OneOfrequest<'a>,
}

impl<'a> MessageRead<'a> for Request<'a> {
    fn from_reader(r: &mut BytesReader, bytes: &'a [u8]) -> Result<Self> {
        let mut msg = Self::default();
        while !r.is_eof() {
            match r.next_tag(bytes) {
                Ok(10) => msg.request = mod_Request::OneOfrequest::get_version(r.read_message::<RequestGetVersion>(bytes)?),
                Ok(18) => msg.request = mod_Request::OneOfrequest::init_swap(r.read_message::<RequestInitSwap>(bytes)?),
                Ok(26) => msg.request = mod_Request::OneOfrequest::init_sell(r.read_message::<RequestInitSell>(bytes)?),
                Ok(34) => msg.request = mod_Request::OneOfrequest::swap(r.read_message::<RequestSwap>(bytes)?),
                Ok(42) => msg.request = mod_Request::OneOfrequest::sell(r.read_message::<RequestSell>(bytes)?),
                Ok(t) => { r.read_unknown(bytes, t)?; }
                Err(e) => return Err(e),
            }
        }
        Ok(msg)
    }
}

impl<'a> MessageWrite for Request<'a> {
    fn get_size(&self) -> usize {
        0
        + match self.request {
            mod_Request::OneOfrequest::get_version(ref m) => 1 + sizeof_len((m).get_size()),
            mod_Request::OneOfrequest::init_swap(ref m) => 1 + sizeof_len((m).get_size()),
            mod_Request::OneOfrequest::init_sell(ref m) => 1 + sizeof_len((m).get_size()),
            mod_Request::OneOfrequest::swap(ref m) => 1 + sizeof_len((m).get_size()),
            mod_Request::OneOfrequest::sell(ref m) => 1 + sizeof_len((m).get_size()),
            mod_Request::OneOfrequest::None => 0,
    }    }

    fn write_message<W: WriterBackend>(&self, w: &mut Writer<W>) -> Result<()> {
        match self.request {            mod_Request::OneOfrequest::get_version(ref m) => { w.write_with_tag(10, |w| w.write_message(m))? },
            mod_Request::OneOfrequest::init_swap(ref m) => { w.write_with_tag(18, |w| w.write_message(m))? },
            mod_Request::OneOfrequest::init_sell(ref m) => { w.write_with_tag(26, |w| w.write_message(m))? },
            mod_Request::OneOfrequest::swap(ref m) => { w.write_with_tag(34, |w| w.write_message(m))? },
            mod_Request::OneOfrequest::sell(ref m) => { w.write_with_tag(42, |w| w.write_message(m))? },
            mod_Request::OneOfrequest::None => {},
    }        Ok(())
    }
}

pub mod mod_Request {

use alloc::vec::Vec;
use super::*;

#[derive(Debug, PartialEq, Clone)]
pub enum OneOfrequest<'a> {
    get_version(RequestGetVersion),
    init_swap(RequestInitSwap),
    init_sell(RequestInitSell),
    swap(RequestSwap<'a>),
    sell(RequestSell<'a>),
    None,
}

impl<'a> Default for OneOfrequest<'a> {
    fn default() -> Self {
        OneOfrequest::None
    }
}

}

#[derive(Debug, Default, PartialEq, Clone)]
pub struct Response<'a> {
    pub response: mod_Response::OneOfresponse<'a>,
}

impl<'a> MessageRead<'a> for Response<'a> {
    fn from_reader(r: &mut BytesReader, bytes: &'a [u8]) -> Result<Self> {
        let mut msg = Self::default();
        while !r.is_eof() {
            match r.next_tag(bytes) {
                Ok(10) => msg.response = mod_Response::OneOfresponse::get_version(r.read_message::<ResponseGetVersion>(bytes)?),
                Ok(18) => msg.response = mod_Response::OneOfresponse::init_swap(r.read_message::<ResponseInitSwap>(bytes)?),
                Ok(26) => msg.response = mod_Response::OneOfresponse::init_sell(r.read_message::<ResponseInitSell>(bytes)?),
                Ok(34) => msg.response = mod_Response::OneOfresponse::swap(r.read_message::<ResponseSwap>(bytes)?),
                Ok(42) => msg.response = mod_Response::OneOfresponse::sell(r.read_message::<ResponseSell>(bytes)?),
                Ok(50) => msg.response = mod_Response::OneOfresponse::error(r.read_message::<ResponseError>(bytes)?),
                Ok(t) => { r.read_unknown(bytes, t)?; }
                Err(e) => return Err(e),
            }
        }
        Ok(msg)
    }
}

impl<'a> MessageWrite for Response<'a> {
    fn get_size(&self) -> usize {
        0
        + match self.response {
            mod_Response::OneOfresponse::get_version(ref m) => 1 + sizeof_len((m).get_size()),
            mod_Response::OneOfresponse::init_swap(ref m) => 1 + sizeof_len((m).get_size()),
            mod_Response::OneOfresponse::init_sell(ref m) => 1 + sizeof_len((m).get_size()),
            mod_Response::OneOfresponse::swap(ref m) => 1 + sizeof_len((m).get_size()),
            mod_Response::OneOfresponse::sell(ref m) => 1 + sizeof_len((m).get_size()),
            mod_Response::OneOfresponse::error(ref m) => 1 + sizeof_len((m).get_size()),
            mod_Response::OneOfresponse::None => 0,
    }    }

    fn write_message<W: WriterBackend>(&self, w: &mut Writer<W>) -> Result<()> {
        match self.response {            mod_Response::OneOfresponse::get_version(ref m) => { w.write_with_tag(10, |w| w.write_message(m))? },
            mod_Response::OneOfresponse::init_swap(ref m) => { w.write_with_tag(18, |w| w.write_message(m))? },
            mod_Response::OneOfresponse::init_sell(ref m) => { w.write_with_tag(26, |w| w.write_message(m))? },
            mod_Response::OneOfresponse::swap(ref m) => { w.write_with_tag(34, |w| w.write_message(m))? },
            mod_Response::OneOfresponse::sell(ref m) => { w.write_with_tag(42, |w| w.write_message(m))? },
            mod_Response::OneOfresponse::error(ref m) => { w.write_with_tag(50, |w| w.write_message(m))? },
            mod_Response::OneOfresponse::None => {},
    }        Ok(())
    }
}

pub mod mod_Response {

use alloc::vec::Vec;
use super::*;

#[derive(Debug, PartialEq, Clone)]
pub enum OneOfresponse<'a> {
    get_version(ResponseGetVersion<'a>),
    init_swap(ResponseInitSwap<'a>),
    init_sell(ResponseInitSell<'a>),
    swap(ResponseSwap<'a>),
    sell(ResponseSell<'a>),
    error(ResponseError<'a>),
    None,
}

impl<'a> Default for OneOfresponse<'a> {
    fn default() -> Self {
        OneOfresponse::None
    }
}

}

