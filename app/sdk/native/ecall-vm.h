#pragma once

#include "../../../vm/src/ecall.h"

/* Native Pointer */
#define NP(_addr) ((const guest_pointer_t){ .addr = (ptrdiff_t)_addr })
