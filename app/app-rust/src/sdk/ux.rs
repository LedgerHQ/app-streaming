use sdk::glyphs::*;
use sdk::{
    ecall_app_loading_start, ecall_app_loading_stop, ecall_bagl_draw_with_context,
    ecall_bagl_hal_draw_bitmap_within_rect, ecall_screen_update, ecall_ux_idle, ecall_wait_button,
};

pub const BAGL_NOFILL: u8 = 0;
pub const BAGL_FILL: u8 = 1;

#[repr(u8)]
pub enum BaglComponentType {
    None = 0,
    Button = 1,
    Label,
    Rectangle,
    Line,
    Icon,
    Circle,
    Labeline,
}

// enum can't be or'ed with int, hence the public module
mod font {
    pub const LUCIDA_CONSOLE_8PX: u16 = 0;
    pub const OPEN_SANS_LIGHT_16_22PX: u16 = 1;
    pub const OPEN_SANS_REGULAR_8_11PX: u16 = 2;
    pub const OPEN_SANS_REGULAR_10_13PX: u16 = 3;
    pub const OPEN_SANS_REGULAR_11_14PX: u16 = 4;
    pub const OPEN_SANS_REGULAR_13_18PX: u16 = 5;
    pub const OPEN_SANS_REGULAR_22_30PX: u16 = 6;
    pub const OPEN_SANS_SEMIBOLD_8_11PX: u16 = 7;
    pub const OPEN_SANS_EXTRABOLD_11PX: u16 = 8;
    pub const OPEN_SANS_LIGHT_16PX: u16 = 9;
    pub const OPEN_SANS_REGULAR_11PX: u16 = 10;
    pub const OPEN_SANS_SEMIBOLD_10_13PX: u16 = 11;
    pub const OPEN_SANS_SEMIBOLD_11_16PX: u16 = 12;
    pub const OPEN_SANS_SEMIBOLD_13_18PX: u16 = 13;

    pub const ALIGNMENT_CENTER: u16 = 0x8000;
}

#[repr(C, packed)]
pub struct BaglComponent {
    pub kind: BaglComponentType,
    pub userid: u8,
    pub x: i16,
    pub y: i16,
    pub width: u16,
    pub height: u16,
    pub stroke: u8,
    pub radius: u8,
    pub fill: u8,
    pub fgcolor: u32,
    pub bgcolor: u32,
    pub font_id: u16,
    pub icon_id: u8,
}

#[derive(PartialEq)]
enum Button {
    Left = 1,
    Right = 2,
    Both = 3,
}

#[repr(C)]
pub struct BaglIconDetails<'a> {
    pub width: usize,
    pub height: usize,
    pub bpp: usize,
    pub colors: &'a [u32],
    pub bitmap: &'a [u8],
}

pub struct BaglRectangle<'a> {
    pub component: &'a BaglComponent,
}

pub struct BaglText<'a> {
    pub component: &'a BaglComponent,
    pub text: &'a str,
}

pub struct BaglIcon<'a> {
    pub x: i32,
    pub y: i32,
    pub icon: &'a BaglIconDetails<'a>,
}

enum BaglElement<'a> {
    Icon(&'a BaglIcon<'a>),
    Rectangle(&'a BaglRectangle<'a>),
    Text(&'a BaglText<'a>),
}

#[derive(Clone, Copy, Eq, PartialEq)]
pub enum UxAction {
    None,
    Validate,
    Reject,
}

pub struct UxItem<'a> {
    pub icon: Option<&'a BaglIconDetails<'a>>,
    pub line1: &'a str,
    pub line2: Option<&'a str>,
    pub action: UxAction,
}

static LAYOUT_BB_LINE1: BaglComponent = BaglComponent {
    kind: BaglComponentType::Labeline,
    userid: 0x10,
    x: 6,
    y: 29,
    width: 116,
    height: 32,
    stroke: 0,
    radius: 0,
    fill: 0,
    fgcolor: 0xFFFFFF,
    bgcolor: 0x000000,
    font_id: font::OPEN_SANS_REGULAR_11PX | font::ALIGNMENT_CENTER,
    icon_id: 0,
};

static LAYOUT_BB_LINE2: BaglComponent = BaglComponent {
    kind: BaglComponentType::Labeline,
    userid: 0x11,
    x: 6,
    y: 43,
    width: 116,
    height: 32,
    stroke: 0,
    radius: 0,
    fill: 0,
    fgcolor: 0xFFFFFF,
    bgcolor: 0x000000,
    font_id: font::OPEN_SANS_REGULAR_11PX | font::ALIGNMENT_CENTER,
    icon_id: 0,
};

static LAYOUT_PN_LINE: BaglComponent = BaglComponent {
    kind: BaglComponentType::Labeline,
    userid: 0x11,
    x: 0,
    y: 44,
    width: 128,
    height: 32,
    stroke: 0,
    radius: 0,
    fill: 0,
    fgcolor: 0xFFFFFF,
    bgcolor: 0x000000,
    font_id: font::OPEN_SANS_EXTRABOLD_11PX | font::ALIGNMENT_CENTER,
    icon_id: 0,
};

static LAYOUT_PBB_LINE1: BaglComponent = BaglComponent {
    kind: BaglComponentType::Labeline,
    userid: 0x10,
    x: 6,
    y: 37,
    width: 116,
    height: 32,
    stroke: 0,
    radius: 0,
    fill: 0,
    fgcolor: 0xFFFFFF,
    bgcolor: 0x000000,
    font_id: font::OPEN_SANS_EXTRABOLD_11PX | font::ALIGNMENT_CENTER,
    icon_id: 0,
};

static LAYOUT_PBB_LINE2: BaglComponent = BaglComponent {
    kind: BaglComponentType::Labeline,
    userid: 0x11,
    x: 6,
    y: 51,
    width: 116,
    height: 32,
    stroke: 0,
    radius: 0,
    fill: 0,
    fgcolor: 0xFFFFFF,
    bgcolor: 0x000000,
    font_id: font::OPEN_SANS_EXTRABOLD_11PX | font::ALIGNMENT_CENTER,
    icon_id: 0,
};

static LAYOUT_ERASE: BaglRectangle = BaglRectangle {
    component: &BaglComponent {
        kind: BaglComponentType::Rectangle,
        userid: 0x00,
        x: 0,
        y: 0,
        width: 128,
        height: 64,
        stroke: 0,
        radius: 0,
        fill: BAGL_FILL,
        fgcolor: 0x000000,
        bgcolor: 0xFFFFFF,
        font_id: 0,
        icon_id: 0,
    },
};

pub fn app_loading_start(status: &str) {
    unsafe { ecall_app_loading_start(status.as_ptr()) }
}

pub fn app_loading_stop() -> bool {
    unsafe { ecall_app_loading_stop() }
}

fn bagl_draw_component(component: &BaglComponent, text: Option<&str>, encoding: u8) {
    let (text, len) = if let Some(p) = text {
        (p.as_ptr(), p.len())
    } else {
        (core::ptr::null(), 0)
    };
    unsafe { ecall_bagl_draw_with_context(component, text, len as u16, encoding) }
}

fn display_icon(icon: &BaglIcon) {
    unsafe {
        ecall_bagl_hal_draw_bitmap_within_rect(
            icon.x,
            icon.y,
            icon.icon.width,
            icon.icon.height,
            icon.icon.colors.as_ptr(),
            icon.icon.bpp,
            icon.icon.bitmap.as_ptr(),
            icon.icon.bpp * icon.icon.width * icon.icon.height,
        );
    }
}

fn screen_update() {
    unsafe { ecall_screen_update() }
}

pub fn ux_idle() {
    unsafe { ecall_ux_idle() }
}

fn display_element(element: &BaglElement) {
    match element {
        BaglElement::Icon(icon) => display_icon(icon),
        BaglElement::Rectangle(rectangle) => bagl_draw_component(rectangle.component, None, 0),
        BaglElement::Text(text) => bagl_draw_component(text.component, Some(text.text), 0),
    }
}

fn display_item(item: &UxItem, first: bool, last: bool) {
    display_element(&BaglElement::Rectangle(&LAYOUT_ERASE));

    if !first {
        display_element(&BaglElement::Icon(&ICON_LEFT));
    }

    if !last {
        display_element(&BaglElement::Icon(&ICON_RIGHT));
    }

    if let Some(icon) = item.icon {
        if let Some(line2) = item.line2 {
            // icon and 2 lines
            let icon_element = BaglIcon { x: 57, y: 10, icon };
            display_element(&BaglElement::Icon(&icon_element));
            let line = BaglText {
                component: &LAYOUT_PBB_LINE1,
                text: item.line1,
            };
            display_element(&BaglElement::Text(&line));
            let line = BaglText {
                component: &LAYOUT_PBB_LINE2,
                text: line2,
            };
            display_element(&BaglElement::Text(&line));
        } else {
            // icon and 1 line
            let icon_element = BaglIcon { x: 57, y: 17, icon };
            display_element(&BaglElement::Icon(&icon_element));
            let line = BaglText {
                component: &LAYOUT_PN_LINE,
                text: item.line1,
            };
            display_element(&BaglElement::Text(&line));
        }
    } else {
        // no icon and 1 or 2 lines
        let line = BaglText {
            component: &LAYOUT_BB_LINE1,
            text: item.line1,
        };
        display_element(&BaglElement::Text(&line));
        if let Some(line2) = item.line2 {
            let line = BaglText {
                component: &LAYOUT_BB_LINE2,
                text: line2,
            };
            display_element(&BaglElement::Text(&line));
        }
    }

    screen_update()
}

fn wait_button() -> Button {
    let n = unsafe { ecall_wait_button() };
    match n {
        1 => Button::Left,
        2 => Button::Right,
        _ => Button::Both,
    }
}

fn wait_input(action: UxAction) -> (Button, bool) {
    loop {
        let button = wait_button();

        if button == Button::Left || button == Button::Right {
            return (button, false);
        } else if action != UxAction::None {
            return (button, action == UxAction::Validate);
        }
    }
}

pub fn ux_validate(items: &[UxItem]) -> bool {
    let mut n = 0;
    let mut refresh = true;

    loop {
        if refresh {
            let first = n == 0;
            let last = n == items.len() - 1;
            display_item(&items[n], first, last);
        }

        let (button, validated) = wait_input(items[n].action);
        match button {
            Button::Left => {
                if n > 0 {
                    n -= 1;
                    refresh = true;
                } else {
                    refresh = false;
                }
            }
            Button::Right => {
                if n + 1 < items.len() {
                    n += 1;
                    refresh = true;
                } else {
                    refresh = false;
                }
            }
            Button::Both => {
                return validated;
            }
        }
    }
}
