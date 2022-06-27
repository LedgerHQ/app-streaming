use sdk::ux::*;

static ICON_LEFT_COLORS: [u32; 2] = [0x00000000, 0x00ffffff];
static ICON_LEFT_BITMAP: [u8; 4] = [0x48, 0x12, 0x42, 0x08];
static GLYPHS_ICON_LEFT: BaglIconDetails = BaglIconDetails {
    width: 4,
    height: 7,
    bpp: 1,
    colors: &ICON_LEFT_COLORS,
    bitmap: &ICON_LEFT_BITMAP,
};
pub static ICON_LEFT: BaglIcon = BaglIcon {
    x: 2,
    y: 28,
    icon: &GLYPHS_ICON_LEFT,
};

static ICON_RIGHT_COLORS: [u32; 2] = [0x00000000, 0x00ffffff];
static ICON_RIGHT_BITMAP: [u8; 4] = [0x21, 0x84, 0x24, 0x01];
static GLYPHS_ICON_RIGHT: BaglIconDetails = BaglIconDetails {
    width: 4,
    height: 7,
    bpp: 1,
    colors: &ICON_RIGHT_COLORS,
    bitmap: &ICON_RIGHT_BITMAP,
};
pub static ICON_RIGHT: BaglIcon = BaglIcon {
    x: 122,
    y: 28,
    icon: &GLYPHS_ICON_RIGHT,
};

static BOILERPLATE_LOGO_COLORS: [u32; 2] = [0x00000000, 0x00ffffff];
static BOILERPLATE_LOGO_BITMAP: [u8; 32] = [
    0xff, 0xff, 0xff, 0xff, 0x7f, 0xfe, 0x7f, 0xfe, 0x3f, 0xfc, 0x3f, 0xfc, 0x01, 0x80, 0x07, 0xe0,
    0x0f, 0xf0, 0x1f, 0xf8, 0x0f, 0xf0, 0x8f, 0xf1, 0xc7, 0xe3, 0xe7, 0xe7, 0xff, 0xff, 0xff, 0xff,
];
pub static BOILERPLATE_LOGO: BaglIconDetails = BaglIconDetails {
    width: 16,
    height: 1,
    bpp: 1,
    colors: &BOILERPLATE_LOGO_COLORS,
    bitmap: &BOILERPLATE_LOGO_BITMAP,
};

static ICON_EYE_COLORS: [u32; 2] = [0x00000000, 0x00ffffff];
static ICON_EYE_BITMAP: [u8; 25] = [
    0x00, 0x00, 0x00, 0x00, 0x1e, 0xe0, 0x1f, 0x1c, 0x0e, 0x03, 0x63, 0x8c, 0x19, 0x63, 0x0c, 0x0c,
    0x87, 0x83, 0x7f, 0x80, 0x07, 0x00, 0x00, 0x00, 0x00,
];
pub static ICON_EYE: BaglIconDetails = BaglIconDetails {
    width: 14,
    height: 14,
    bpp: 1,
    colors: &ICON_EYE_COLORS,
    bitmap: &ICON_EYE_BITMAP,
};

static ICON_VALIDATE_COLORS: [u32; 2] = [0x00000000, 0x00ffffff];
static ICON_VALIDATE_BITMAP: [u8; 25] = [
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x00, 0x38, 0x00, 0x67, 0xe0, 0x38, 0x1c, 0x9c, 0x03,
    0x7e, 0x00, 0x0f, 0x80, 0x01, 0x00, 0x00, 0x00, 0x00,
];
pub static ICON_VALIDATE: BaglIconDetails = BaglIconDetails {
    width: 14,
    height: 14,
    bpp: 1,
    colors: &ICON_VALIDATE_COLORS,
    bitmap: &ICON_VALIDATE_BITMAP,
};

static ICON_CROSSMARK_COLORS: [u32; 2] = [0x00000000, 0x00ffffff];
static ICON_CROSSMARK_BITMAP: [u8; 25] = [
    0x00, 0x80, 0x01, 0xe6, 0xc0, 0x71, 0x38, 0x38, 0x07, 0xfc, 0x00, 0x1e, 0x80, 0x07, 0xf0, 0x03,
    0xce, 0xc1, 0xe1, 0x38, 0x70, 0x06, 0x18, 0x00, 0x00,
];
pub static ICON_CROSSMARK: BaglIconDetails = BaglIconDetails {
    width: 14,
    height: 14,
    bpp: 1,
    colors: &ICON_CROSSMARK_COLORS,
    bitmap: &ICON_CROSSMARK_BITMAP,
};

/*
static ICON_validate_colors[u32: 2] = [
    0x00000000,
    0x00ffffff,
];

static ICON_dashboard_colors[] = {
    0x00000000,
    0x00ffffff,
};

static icon_dashboard_bitmap: [u8; 4] = [
    0xe0, 0x01, 0xfe, 0xc1, 0xff, 0x38, 0x70, 0x06, 0xd8, 0x79, 0x7e, 0x9e, 0x9f,
    0xe7, 0xe7, 0xb9, 0x01, 0xe6, 0xc0, 0xf1, 0x3f, 0xf8, 0x07, 0x78, 0x00,
};

pub static icon_dashboard: BaglIconDetails = BaglIconDetails { GLYPH_icon_dashboard_WIDTH,
                                               GLYPH_icon_dashboard_HEIGHT, 1,
                                               icon_dashboard_colors, icon_dashboard_bitmap };

static ICON_dashboard_x_colors[] = {
    0x00000000,
    0x00ffffff,
};

static icon_dashboard_x_bitmap: [u8; 4] = [
    0x00, 0x00, 0x00, 0x00, 0x0c, 0x80, 0x07, 0xf0, 0x03, 0xfe, 0xc1, 0xff, 0xf0,
    0x3f, 0xf0, 0x03, 0xcc, 0x00, 0x33, 0xc0, 0x0c, 0x00, 0x00, 0x00, 0x00,
};

pub static icon_dashboard_x: BaglIconDetails = BaglIconDetails { GLYPH_icon_dashboard_x_WIDTH,
                                                 GLYPH_icon_dashboard_x_HEIGHT, 1,
                                                 icon_dashboard_x_colors,
                                                 icon_dashboard_x_bitmap };


static ICON_down_colors[] = {
    0x00000000,
    0x00ffffff,
};

static icon_down_bitmap: [u8; 4] = [
    0x41,
    0x11,
    0x05,
    0x01,
};

pub static icon_down: BaglIconDetails = BaglIconDetails { GLYPH_icon_down_WIDTH, GLYPH_icon_down_HEIGHT, 1,
                                          icon_down_colors, icon_down_bitmap };

static ICON_up_colors[] = {
    0x00000000,
    0x00ffffff,
};

static icon_up_bitmap: [u8; 4] = [
    0x08,
    0x8a,
    0x28,
    0x08,
};

pub static icon_up: BaglIconDetails = BaglIconDetails { GLYPH_icon_up_WIDTH, GLYPH_icon_up_HEIGHT, 1,
                                        icon_up_colors, icon_up_bitmap };

static ICON_warning_colors[] = {
    0x00000000,
    0x00ffffff,
};

static icon_warning_bitmap: [u8; 4] = [
    0x00, 0x00, 0x30, 0x00, 0x0c, 0x80, 0x07, 0x20, 0x01, 0xcc, 0x00, 0x33, 0xe0,
    0x1c, 0x38, 0x07, 0xff, 0xc3, 0xf3, 0xf8, 0x7c, 0xfe, 0x1f, 0x00, 0x00,
};

pub static icon_warning: BaglIconDetails = BaglIconDetails { GLYPH_icon_warning_WIDTH, GLYPH_icon_warning_HEIGHT, 1,
                                             icon_warning_colors, icon_warning_bitmap };
*/
