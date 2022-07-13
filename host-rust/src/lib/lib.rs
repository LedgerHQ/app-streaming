#![allow(dead_code)] // XXX
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

use cpython::{py_fn, py_module_initializer, PyBytes, PyNone, PyObject, PyResult, Python};

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
    Ok(())
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
