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
    py_class, py_fn, py_module_initializer, PyBytes, PyNone, PyObject, PyResult, Python,
};
use std::cell::RefCell;

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
    data stream: RefCell<stream::Stream>;

    def __new__(_cls, path: &str, comm: &PyObject) -> PyResult<Stream> {
        let comm = comm::Comm::new(py, comm);
        Self::create_instance(py, RefCell::new(stream::Stream::new(path, comm)))
    }

    def exchange(&self, recv_buffer: &[u8]) -> PyResult<PyNone> {
        let mut stream = self.stream(py).borrow_mut();
        stream.exchange(recv_buffer);
        Ok(PyNone)
    }
});

fn get_pubkey_py(py: Python, path: &str, comm: &PyObject) -> PyResult<PyBytes> {
    let app = app::App::from_zip(path);
    let comm = comm::Comm::new(py, comm);
    let pubkey = app.get_pubkey(&comm);
    Ok(PyBytes::new(py, &pubkey))
}

fn device_sign_app_py(py: Python, path: &str, comm: &PyObject) -> PyResult<PyNone> {
    let mut app = app::App::from_zip(path);
    let comm = comm::Comm::new(py, comm);
    app.device_sign_app(&comm);
    Ok(PyNone)
}
