use reqwest::blocking::Response;
use reqwest::header::{HeaderMap, HeaderValue, ACCEPT, CONTENT_TYPE};
use serde::de::DeserializeOwned;
use serde::{Deserialize, Serialize};
use std::ops::Deref;
use std::time::Duration;

#[derive(Debug, Serialize)]
struct HttpRequest<'a> {
    data: &'a str,
}

#[derive(Deserialize, Debug)]
struct HttpResponse {
    data: String,
    error: Option<String>,
}

const APDU_URL: &str = "http://127.0.0.1:5000/apdu";

pub fn exchange(data: &[u8]) -> Vec<u8> {
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
