#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "ecall.h"

void app_loading_inc_counter(void);
void app_loading_update_ui(bool host_exchange);
