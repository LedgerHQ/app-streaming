#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "sdk.h"
#include "ux.h"

#define BUTTON_LEFT  (1 << 0)
#define BUTTON_RIGHT (1 << 1)

static int wait_input(enum ux_action_e action, bool *validated)
{
    int button;

    while (1) {
        button = wait_button();

        if (button == BUTTON_LEFT || button == BUTTON_RIGHT) {
            break;
        }

        if (button == (BUTTON_LEFT | BUTTON_RIGHT) && action != UX_ACTION_NONE) {
            *validated = (action == UX_ACTION_VALIDATE) ? true : false;
            break;
        }
    }

    return button;
}

bool ux_validate(struct ux_item_s *items, size_t count)
{
    unsigned int n = 0;
    bool refresh = true;

    while (1) {
        if (refresh) {
            bool first = (n == 0);
            bool last = (n == count - 1);
            display_item(&items[n], first, last);
        }

        bool validated;
        int button = wait_input(items[n].action, &validated);
        if (button == BUTTON_LEFT) {
            if (n > 0) {
                n--;
                refresh = true;
            } else {
                refresh = false;
            }
        } else if (button == BUTTON_RIGHT) {
            if (n + 1 < count) {
                n++;
                refresh = true;
            } else {
                refresh = false;
            }
        } else {
            return validated;
        }
    }
}
