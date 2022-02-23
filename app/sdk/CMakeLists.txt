if (NOT NATIVE)
  add_library(sdk
    risc-v/ecall-riscv.c
    risc-v/start.S
    risc-v/std.c
    sdk.c)
else()
  add_library(sdk
    native/ecall-native.c
    sdk.c)
endif()
