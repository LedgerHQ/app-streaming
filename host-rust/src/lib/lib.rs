#![allow(dead_code)] // XXX
#![allow(clippy::manual_strip)] // https://github.com/dgrunwald/rust-cpython/issues/245#934934
#![allow(clippy::zero_ptr)]
#![feature(proc_macro_hygiene)]

extern crate cpython;
extern crate crypto;
extern crate hex;
extern crate hex_literal;
extern crate reqwest;
extern crate serde;
extern crate zip;

pub mod app;
pub mod comm;
pub mod manifest;
mod merkletree;
pub mod serialization;
pub mod stream;

use cpython::{
    py_class, py_fn, py_module_initializer, ObjectProtocol, PyBytes, PyClone, PyNone, PyObject,
    PyResult, Python,
};
use std::cell::RefCell;

use comm::Comm;

py_module_initializer!(libstreaming, |py, m| {
    m.add(py, "__doc__", "This module is implemented in Rust.")?;
    m.add(
        py,
        "get_pubkey",
        py_fn!(py, get_pubkey_py(path: &str, comm: &PyObject)),
    )?;
    m.add(
        py,
        "device_sign_app",
        py_fn!(py, device_sign_app_py(path: &str, comm: &PyObject)),
    )?;

    m.add_class::<Stream>(py)?;

    Ok(())
});

py_class!(class Stream |py| {
    data stream: RefCell<stream::Stream<PyComm>>;

    def __new__(_cls, path: &str, comm: &PyObject) -> PyResult<Stream> {
        let comm = PyComm::new(py, comm);
        Self::create_instance(py, RefCell::new(stream::Stream::new(path, comm)))
    }

    def exchange(&self, recv_buffer: &[u8]) -> PyResult<PyBytes> {
        let mut stream = self.stream(py).borrow_mut();
        if let Some(data) = stream.exchange(recv_buffer) {
            Ok(PyBytes::new(py, &data))
        } else {
            Ok(PyBytes::new(py, &[0u8; 0]))
        }
    }
});

fn get_pubkey_py(py: Python, path: &str, comm: &PyObject) -> PyResult<PyBytes> {
    let app = app::App::from_zip(path);
    let comm = PyComm::new(py, comm);
    let pubkey = app.get_pubkey(&comm);
    Ok(PyBytes::new(py, &pubkey))
}

fn device_sign_app_py(py: Python, path: &str, comm: &PyObject) -> PyResult<PyNone> {
    let mut app = app::App::from_zip(path);
    let comm = PyComm::new(py, comm);
    app.device_sign_app(&comm);
    Ok(PyNone)
}

pub struct PyComm {
    comm: PyObject,
}

impl PyComm {
    fn new(py: Python, comm: &PyObject) -> PyComm {
        PyComm {
            comm: comm.clone_ref(py),
        }
    }
}

impl Comm for PyComm {
    fn exchange_apdu(&self, apdu: &[u8]) -> Vec<u8> {
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
}
