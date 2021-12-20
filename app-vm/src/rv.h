#ifndef RV_H
#define RV_H

#include <stdint.h>
#include "types.h"

enum rv_status {
    RV_STATUS_OK,
    RV_STATUS_INVALID_INST,
    RV_STATUS_MEMORY_ERROR
};

#include "rv_cpu.h"

#endif // RV_H
