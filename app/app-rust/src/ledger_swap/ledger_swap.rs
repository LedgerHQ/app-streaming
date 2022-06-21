// Automatically generated rust module for 'swap.proto' file

#![allow(non_snake_case)]
#![allow(non_upper_case_globals)]
#![allow(non_camel_case_types)]
#![allow(unused_imports)]
#![allow(unknown_lints)]
#![allow(clippy::all)]
#![cfg_attr(rustfmt, rustfmt_skip)]

//use std::borrow::Cow;
use alloc::borrow::Cow;
use quick_protobuf::{MessageRead, MessageWrite, BytesReader, Writer, WriterBackend, Result};
use quick_protobuf::sizeofs::*;
use super::*;

#[derive(Debug, Default, PartialEq, Clone)]
pub struct NewTransactionResponse<'a> {
    pub payin_address: Cow<'a, str>,
    pub payin_extra_id: Cow<'a, str>,
    pub refund_address: Cow<'a, str>,
    pub refund_extra_id: Cow<'a, str>,
    pub payout_address: Cow<'a, str>,
    pub payout_extra_id: Cow<'a, str>,
    pub currency_from: Cow<'a, str>,
    pub currency_to: Cow<'a, str>,
    pub amount_to_provider: Cow<'a, [u8]>,
    pub amount_to_wallet: Cow<'a, [u8]>,
    pub device_transaction_id: Cow<'a, str>,
}

impl<'a> MessageRead<'a> for NewTransactionResponse<'a> {
    fn from_reader(r: &mut BytesReader, bytes: &'a [u8]) -> Result<Self> {
        let mut msg = Self::default();
        while !r.is_eof() {
            match r.next_tag(bytes) {
                Ok(10) => msg.payin_address = r.read_string(bytes).map(Cow::Borrowed)?,
                Ok(18) => msg.payin_extra_id = r.read_string(bytes).map(Cow::Borrowed)?,
                Ok(26) => msg.refund_address = r.read_string(bytes).map(Cow::Borrowed)?,
                Ok(34) => msg.refund_extra_id = r.read_string(bytes).map(Cow::Borrowed)?,
                Ok(42) => msg.payout_address = r.read_string(bytes).map(Cow::Borrowed)?,
                Ok(50) => msg.payout_extra_id = r.read_string(bytes).map(Cow::Borrowed)?,
                Ok(58) => msg.currency_from = r.read_string(bytes).map(Cow::Borrowed)?,
                Ok(66) => msg.currency_to = r.read_string(bytes).map(Cow::Borrowed)?,
                Ok(74) => msg.amount_to_provider = r.read_bytes(bytes).map(Cow::Borrowed)?,
                Ok(82) => msg.amount_to_wallet = r.read_bytes(bytes).map(Cow::Borrowed)?,
                Ok(90) => msg.device_transaction_id = r.read_string(bytes).map(Cow::Borrowed)?,
                Ok(t) => { r.read_unknown(bytes, t)?; }
                Err(e) => return Err(e),
            }
        }
        Ok(msg)
    }
}

impl<'a> MessageWrite for NewTransactionResponse<'a> {
    fn get_size(&self) -> usize {
        0
        + if self.payin_address == "" { 0 } else { 1 + sizeof_len((&self.payin_address).len()) }
        + if self.payin_extra_id == "" { 0 } else { 1 + sizeof_len((&self.payin_extra_id).len()) }
        + if self.refund_address == "" { 0 } else { 1 + sizeof_len((&self.refund_address).len()) }
        + if self.refund_extra_id == "" { 0 } else { 1 + sizeof_len((&self.refund_extra_id).len()) }
        + if self.payout_address == "" { 0 } else { 1 + sizeof_len((&self.payout_address).len()) }
        + if self.payout_extra_id == "" { 0 } else { 1 + sizeof_len((&self.payout_extra_id).len()) }
        + if self.currency_from == "" { 0 } else { 1 + sizeof_len((&self.currency_from).len()) }
        + if self.currency_to == "" { 0 } else { 1 + sizeof_len((&self.currency_to).len()) }
        + if self.amount_to_provider == Cow::Borrowed(b"") { 0 } else { 1 + sizeof_len((&self.amount_to_provider).len()) }
        + if self.amount_to_wallet == Cow::Borrowed(b"") { 0 } else { 1 + sizeof_len((&self.amount_to_wallet).len()) }
        + if self.device_transaction_id == "" { 0 } else { 1 + sizeof_len((&self.device_transaction_id).len()) }
    }

    fn write_message<W: WriterBackend>(&self, w: &mut Writer<W>) -> Result<()> {
        if self.payin_address != "" { w.write_with_tag(10, |w| w.write_string(&**&self.payin_address))?; }
        if self.payin_extra_id != "" { w.write_with_tag(18, |w| w.write_string(&**&self.payin_extra_id))?; }
        if self.refund_address != "" { w.write_with_tag(26, |w| w.write_string(&**&self.refund_address))?; }
        if self.refund_extra_id != "" { w.write_with_tag(34, |w| w.write_string(&**&self.refund_extra_id))?; }
        if self.payout_address != "" { w.write_with_tag(42, |w| w.write_string(&**&self.payout_address))?; }
        if self.payout_extra_id != "" { w.write_with_tag(50, |w| w.write_string(&**&self.payout_extra_id))?; }
        if self.currency_from != "" { w.write_with_tag(58, |w| w.write_string(&**&self.currency_from))?; }
        if self.currency_to != "" { w.write_with_tag(66, |w| w.write_string(&**&self.currency_to))?; }
        if self.amount_to_provider != Cow::Borrowed(b"") { w.write_with_tag(74, |w| w.write_bytes(&**&self.amount_to_provider))?; }
        if self.amount_to_wallet != Cow::Borrowed(b"") { w.write_with_tag(82, |w| w.write_bytes(&**&self.amount_to_wallet))?; }
        if self.device_transaction_id != "" { w.write_with_tag(90, |w| w.write_string(&**&self.device_transaction_id))?; }
        Ok(())
    }
}

#[derive(Debug, Default, PartialEq, Clone)]
pub struct UDecimal<'a> {
    pub coefficient: Cow<'a, [u8]>,
    pub exponent: u32,
}

impl<'a> MessageRead<'a> for UDecimal<'a> {
    fn from_reader(r: &mut BytesReader, bytes: &'a [u8]) -> Result<Self> {
        let mut msg = Self::default();
        while !r.is_eof() {
            match r.next_tag(bytes) {
                Ok(10) => msg.coefficient = r.read_bytes(bytes).map(Cow::Borrowed)?,
                Ok(16) => msg.exponent = r.read_uint32(bytes)?,
                Ok(t) => { r.read_unknown(bytes, t)?; }
                Err(e) => return Err(e),
            }
        }
        Ok(msg)
    }
}

impl<'a> MessageWrite for UDecimal<'a> {
    fn get_size(&self) -> usize {
        0
        + if self.coefficient == Cow::Borrowed(b"") { 0 } else { 1 + sizeof_len((&self.coefficient).len()) }
        + if self.exponent == 0u32 { 0 } else { 1 + sizeof_varint(*(&self.exponent) as u64) }
    }

    fn write_message<W: WriterBackend>(&self, w: &mut Writer<W>) -> Result<()> {
        if self.coefficient != Cow::Borrowed(b"") { w.write_with_tag(10, |w| w.write_bytes(&**&self.coefficient))?; }
        if self.exponent != 0u32 { w.write_with_tag(16, |w| w.write_uint32(*&self.exponent))?; }
        Ok(())
    }
}

#[derive(Debug, Default, PartialEq, Clone)]
pub struct NewSellResponse<'a> {
    pub trader_email: Cow<'a, str>,
    pub in_currency: Cow<'a, str>,
    pub in_amount: Cow<'a, [u8]>,
    pub in_address: Cow<'a, str>,
    pub out_currency: Cow<'a, str>,
    pub out_amount: Option<ledger_swap::UDecimal<'a>>,
    pub device_transaction_id: Cow<'a, [u8]>,
}

impl<'a> MessageRead<'a> for NewSellResponse<'a> {
    fn from_reader(r: &mut BytesReader, bytes: &'a [u8]) -> Result<Self> {
        let mut msg = Self::default();
        while !r.is_eof() {
            match r.next_tag(bytes) {
                Ok(10) => msg.trader_email = r.read_string(bytes).map(Cow::Borrowed)?,
                Ok(18) => msg.in_currency = r.read_string(bytes).map(Cow::Borrowed)?,
                Ok(26) => msg.in_amount = r.read_bytes(bytes).map(Cow::Borrowed)?,
                Ok(34) => msg.in_address = r.read_string(bytes).map(Cow::Borrowed)?,
                Ok(42) => msg.out_currency = r.read_string(bytes).map(Cow::Borrowed)?,
                Ok(50) => msg.out_amount = Some(r.read_message::<ledger_swap::UDecimal>(bytes)?),
                Ok(58) => msg.device_transaction_id = r.read_bytes(bytes).map(Cow::Borrowed)?,
                Ok(t) => { r.read_unknown(bytes, t)?; }
                Err(e) => return Err(e),
            }
        }
        Ok(msg)
    }
}

impl<'a> MessageWrite for NewSellResponse<'a> {
    fn get_size(&self) -> usize {
        0
        + if self.trader_email == "" { 0 } else { 1 + sizeof_len((&self.trader_email).len()) }
        + if self.in_currency == "" { 0 } else { 1 + sizeof_len((&self.in_currency).len()) }
        + if self.in_amount == Cow::Borrowed(b"") { 0 } else { 1 + sizeof_len((&self.in_amount).len()) }
        + if self.in_address == "" { 0 } else { 1 + sizeof_len((&self.in_address).len()) }
        + if self.out_currency == "" { 0 } else { 1 + sizeof_len((&self.out_currency).len()) }
        + self.out_amount.as_ref().map_or(0, |m| 1 + sizeof_len((m).get_size()))
        + if self.device_transaction_id == Cow::Borrowed(b"") { 0 } else { 1 + sizeof_len((&self.device_transaction_id).len()) }
    }

    fn write_message<W: WriterBackend>(&self, w: &mut Writer<W>) -> Result<()> {
        if self.trader_email != "" { w.write_with_tag(10, |w| w.write_string(&**&self.trader_email))?; }
        if self.in_currency != "" { w.write_with_tag(18, |w| w.write_string(&**&self.in_currency))?; }
        if self.in_amount != Cow::Borrowed(b"") { w.write_with_tag(26, |w| w.write_bytes(&**&self.in_amount))?; }
        if self.in_address != "" { w.write_with_tag(34, |w| w.write_string(&**&self.in_address))?; }
        if self.out_currency != "" { w.write_with_tag(42, |w| w.write_string(&**&self.out_currency))?; }
        if let Some(ref s) = self.out_amount { w.write_with_tag(50, |w| w.write_message(s))?; }
        if self.device_transaction_id != Cow::Borrowed(b"") { w.write_with_tag(58, |w| w.write_bytes(&**&self.device_transaction_id))?; }
        Ok(())
    }
}

