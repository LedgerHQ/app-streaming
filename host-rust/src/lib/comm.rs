use cpython::{ObjectProtocol, PyBytes, PyClone, PyObject, Python};
use std::convert::TryInto;

#[derive(Debug, Eq, PartialEq)]
pub enum Status {
    RequestPage = 0x6101,
    RequestHmac = 0x6102,
    RequestProof = 0x6103,
    CommitPage = 0x6201,
    CommitHmac = 0x6202,
    SendBuffer = 0x6301,
    RecvBuffer = 0x6401,
    Exit = 0x6501,
    Fatal = 0x6601,
    RequestManifest = 0x6701,
    RequestAppPage = 0x6801,
    RequestAppHmac = 0x6802,
    Success = 0x9000,
}

impl From<u16> for Status {
    fn from(n: u16) -> Self {
        match n {
            0x6101 => Status::RequestPage,
            0x6102 => Status::RequestHmac,
            0x6103 => Status::RequestProof,
            0x6201 => Status::CommitPage,
            0x6202 => Status::CommitHmac,
            0x6301 => Status::SendBuffer,
            0x6401 => Status::RecvBuffer,
            0x6501 => Status::Exit,
            0x6601 => Status::Fatal,
            0x6701 => Status::RequestManifest,
            0x6801 => Status::RequestAppPage,
            0x6802 => Status::RequestAppHmac,
            0x9000 => Status::Success,
            _ => panic!("invalid APDU status"),
        }
    }
}

pub struct Apdu<'a> {
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

pub struct Comm {
    comm: PyObject,
}

impl Comm {
    pub fn new(py: Python, comm: &PyObject) -> Comm {
        Comm {
            comm: comm.clone_ref(py),
        }
    }

    pub fn exchange_apdu(&self, apdu: &[u8]) -> Vec<u8> {
        let gil = Python::acquire_gil();
        let py = gil.python();
        let data = PyBytes::new(py, apdu);
        let result: PyBytes = self
            .comm
            .call_method(py, "_exchange", (&data,), None)
            .unwrap()
            .extract(py)
            .unwrap();
        result.data(py).to_vec()
    }

    pub fn exchange(
        &self,
        ins: u8,
        data: &[u8],
        p1: Option<u8>,
        p2: Option<u8>,
        cla: Option<u8>,
    ) -> (Status, Vec<u8>) {
        let apdu = Apdu {
            cla: cla.unwrap_or(0x12),
            ins,
            p1: p1.unwrap_or(0x00),
            p2: p2.unwrap_or(0x00),
            lc: data.len().try_into().unwrap(),
            data,
        };
        let data = self.exchange_apdu(&apdu.to_bytes());
        assert!(data.len() >= 2);
        let status = u16::from_be_bytes(data[data.len() - 2..].try_into().unwrap());
        (Status::from(status), data[..data.len() - 2].to_vec())
    }
}
