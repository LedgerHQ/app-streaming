// https://www.reddit.com/r/rust/comments/dw2vb3/convert_from_u8_to_generic_sized_struct/
fn u8_slice_as_any<T: Sized>(buf: &[u8]) -> T {
    let p: *const T = buf.as_ptr() as *const T;
    let a_ref: &T = unsafe { &*p };
    unsafe { std::mem::transmute_copy::<T, T>(a_ref) }
}

pub trait Deserialize {
    fn from_bytes(data: &[u8]) -> Self
    where
        Self: Sized,
    {
        assert!(data.len() >= ::std::mem::size_of::<Self>());
        u8_slice_as_any(data)
    }
}

// From https://stackoverflow.com/questions/28127165/how-to-convert-struct-to-u8
unsafe fn any_as_u8_slice<T: Sized>(p: &T) -> &[u8] {
    ::std::slice::from_raw_parts((p as *const T) as *const u8, ::std::mem::size_of::<T>())
}

pub trait Serialize {
    fn to_vec(&self) -> Vec<u8>
    where
        Self: Sized,
    {
        let slice = unsafe { any_as_u8_slice(self) };
        slice.to_vec()
    }
}
