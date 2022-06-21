use core::alloc::{GlobalAlloc, Layout};

extern "C" {
    fn malloc(s: usize) -> *mut u8;
    fn free(ptr: *mut u8);
}

pub struct CAlloc;
unsafe impl GlobalAlloc for CAlloc {
    unsafe fn alloc(&self, layout: Layout) -> *mut u8 {
        malloc(layout.size())
    }
    unsafe fn dealloc(&self, ptr: *mut u8, _layout: Layout) {
        free(ptr);
    }
}
