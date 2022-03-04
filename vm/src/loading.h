#pragma once

#include <stdbool.h>
#include <stdint.h>

void app_loading_inc_counter(void);
void app_loading_update_ui(bool host_exchange);
void sys_app_loading_start(uint32_t status_addr);
bool sys_app_loading_stop(void);
