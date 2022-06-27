use sdk::glyphs::{ICON_CROSSMARK, ICON_EYE, ICON_VALIDATE};
use sdk::ux::*;

pub fn sign_tx_validation(send_amount: &str, get_amount: &str, fees: &str) -> bool {
    let swap_ui: [UxItem; 6] = [
        UxItem {
            icon: Some(&ICON_EYE),
            line1: "Review",
            line2: Some("Transaction"),
            action: UxAction::None,
        },
        UxItem {
            icon: None,
            line1: "Send",
            line2: Some(send_amount),
            action: UxAction::None,
        },
        UxItem {
            icon: None,
            line1: "Get",
            line2: Some(get_amount),
            action: UxAction::None,
        },
        UxItem {
            icon: None,
            line1: "Fees",
            line2: Some(fees),
            action: UxAction::None,
        },
        UxItem {
            icon: Some(&ICON_VALIDATE),
            line1: "Accept",
            line2: Some("and send"),
            action: UxAction::Validate,
        },
        UxItem {
            icon: Some(&ICON_CROSSMARK),
            line1: "Reject",
            line2: None,
            action: UxAction::Reject,
        },
    ];

    app_loading_stop();
    ux_validate(&swap_ui)
}
