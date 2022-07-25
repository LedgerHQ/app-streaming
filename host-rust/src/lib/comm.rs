use cpython::{ObjectProtocol, PyBytes, PyClone, PyObject, Python};
use std::convert::TryInto;

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
    ) -> (u16, Vec<u8>) {
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
        (status, data[..data.len() - 2].to_vec())
    }
}
