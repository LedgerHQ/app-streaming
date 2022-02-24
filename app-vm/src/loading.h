#pragma once

#include <stdbool.h>

void app_loading_inc_counter(void);
void app_loading_update_ui(bool host_exchange);
void app_loading_start(void);
bool app_loading_stop(void);
