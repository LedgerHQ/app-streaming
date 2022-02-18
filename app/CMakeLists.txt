cmake_minimum_required(VERSION 3.10)

project(RISC-V-apps C ASM)

if (CMAKE_C_COMPILER_ID MATCHES "(Apple)?[Cc]lang")
  message(STATUS "Using clang compiler")
  set(CMAKE_C_COMPILER clang)
  add_compile_options(--target=riscv32 -Oz)
else ()
  set(CMAKE_C_COMPILER /usr/xcc/riscv32-unknown-linux-gnu/bin/riscv32-unknown-linux-gnu-gcc)
  add_compile_options(-mno-div -mno-fdiv -mstrict-align -Os)
  message(STATUS "Using default compiler ${CMAKE_C_COMPILER}")
endif ()

add_compile_options(-march=rv32g -Wall)
add_link_options(-z noseparate-code)

include_directories(/usr/local/riscv32-unknown-linux-gnu/include/ ./ sdk/)
link_directories(/usr/local/riscv32-unknown-linux-gnu/lib)
link_libraries(c gcc)

add_subdirectory(sdk)
add_subdirectory(ux)

add_subdirectory(app-boilerplate)
add_subdirectory(app-sha256)
add_subdirectory(app-wip)
