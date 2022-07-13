use cpython::{ObjectProtocol, PyBytes, PyObject, Python};

pub struct Comm<'a> {
    py: Python<'a>,
    comm: PyObject,
}

impl Comm<'_> {
    pub fn new<'a>(py: Python<'a>, comm: PyObject) -> Comm<'a> {
        Comm { py, comm }
    }

    pub fn exchange(&self, data: &[u8]) -> Vec<u8> {
        let data = PyBytes::new(self.py, data);
        let result: PyBytes = self
            .comm
            .call_method(self.py, "_exchange", (&data,), None)
            .unwrap()
            .extract(self.py)
            .unwrap();
        result.data(self.py).to_vec()
    }
}
