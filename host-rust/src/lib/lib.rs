#![feature(proc_macro_hygiene)]

extern crate cpython;
extern crate crypto;
extern crate hex;
extern crate hex_literal;
extern crate reqwest;
extern crate serde;
extern crate zip;

pub mod app;
pub mod manifest;
pub mod serialization;
pub mod speculos;

use cpython::{py_fn, py_module_initializer, ObjectProtocol, PyBytes, PyObject, PyResult, Python};
use std::convert::TryInto;

// PYTHONPATH=$(pwd)/target/debug/ python3

// add bindings to the generated python module
// N.B: names: "rust2py" must be the name of the `.so` or `.pyd` file
py_module_initializer!(libstreaming, |py, m| {
    m.add(py, "__doc__", "This module is implemented in Rust.")?;
    m.add(
        py,
        "sum_as_string",
        py_fn!(py, sum_as_string_py(a: i64, b: i64)),
    )?;
    m.add(
        py,
        "get_pubkey",
        py_fn!(py, get_pubkey_py(path: &str, comm: &PyObject)),
    )?;
    Ok(())
});

// logic implemented as a normal rust function
fn sum_as_string(a: i64, b: i64) -> String {
    format!("{}", a + b).to_string()
}

// rust-cpython aware function. All of our python interface could be
// declared in a separate module.
// Note that the py_fn!() macro automatically converts the arguments from
// Python objects to Rust values; and the Rust return value back into a Python object.
fn sum_as_string_py(_: Python, a: i64, b: i64) -> PyResult<String> {
    let out = sum_as_string(a, b);
    Ok(out)
}

fn get_pubkey_py(py: Python, path: &str, comm: &PyObject) -> PyResult<PyBytes> {
    let app_hash = [0x61u8; 32];
    let apdu = speculos::build_apdu(0x10, &app_hash, None, None, Some(0x34));
    let apdu = PyBytes::new(py, &apdu);
    let result: PyBytes = comm.call_method(py, "_exchange", (&apdu,), None).unwrap().extract(py).unwrap();
    Ok(result)
}
