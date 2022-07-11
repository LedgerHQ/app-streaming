use reqwest::blocking::Response;
use reqwest::header::{HeaderMap, HeaderValue, ACCEPT, CONTENT_TYPE};
use serde::{Deserialize, Serialize};
use std::convert::TryInto;
use std::time::Duration;

struct Apdu<'a> {
    cla: u8,
    ins: u8,
    p1: u8,
    p2: u8,
    lc: u8,
    data: &'a [u8],
}

impl Apdu<'_> {
    pub fn to_bytes(&self) -> Vec<u8> {
        let mut bytes = vec![0; 5 + self.data.len()];
        bytes[0] = self.cla;
        bytes[1] = self.ins;
        bytes[2] = self.p1;
        bytes[3] = self.p2;
        bytes[4] = self.lc;
        bytes[5..].copy_from_slice(self.data);
        bytes
    }
}

#[derive(Debug, Serialize)]
struct HttpRequest<'a> {
    data: &'a str,
}

#[derive(Deserialize, Debug)]
struct HttpResponse {
    data: String,
    //error: Option<String>,
}

const APDU_URL: &str = "http://127.0.0.1:5000/apdu";

fn exchange_(data: &[u8]) -> Vec<u8> {
    let mut headers = HeaderMap::new();
    headers.insert(ACCEPT, HeaderValue::from_static("application/json"));
    headers.insert(CONTENT_TYPE, HeaderValue::from_static("application/json"));

    let request = HttpRequest {
        data: &hex::encode(data),
    };

    let response: Response = reqwest::blocking::Client::new()
        .post(APDU_URL)
        .headers(headers)
        .timeout(Duration::from_secs(5))
        .json(&request)
        .send()
        .unwrap();

    let body: HttpResponse = response.json().unwrap();
    hex::decode(body.data).unwrap()
}

pub fn exchange(
    ins: u8,
    data: &[u8],
    p1: Option<u8>,
    p2: Option<u8>,
    cla: Option<u8>,
) -> (u16, Vec<u8>) {
    let apdu = Apdu {
        cla: cla.unwrap_or(0x12),
        ins,
        p1: p1.unwrap_or(0x00),
        p2: p2.unwrap_or(0x00),
        lc: data.len().try_into().unwrap(),
        data,
    };
    let data = exchange_(&apdu.to_bytes());
    assert!(data.len() >= 2);
    let status = u16::from_be_bytes(data[data.len() - 2..].try_into().unwrap());
    (status, data[..data.len() - 2].to_vec())
}
