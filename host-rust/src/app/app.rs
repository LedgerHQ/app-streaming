extern crate streaming;

use streaming::app::{device_sign_app, get_pubkey, App};

pub fn main() {
    let mut app = App::from_zip("/tmp/app.zip");
    let pubkey = get_pubkey(&app);
    println!("{:?}", pubkey);
    device_sign_app(&mut app);
}
