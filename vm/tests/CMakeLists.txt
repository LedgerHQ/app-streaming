cmake_minimum_required(VERSION 3.10)

project(VMtests)

include(CTest)

add_executable(test_memory
  ../src/lfsr.c
  mock_stream.c
  test_memory.c
  )

add_executable(vm
  ../src/cpu.c
  vm.c
  )

target_compile_options(test_memory PRIVATE -Werror -DNATIVE)

target_compile_options(vm PRIVATE -Werror -DNATIVE)
target_include_directories(vm PUBLIC ../src/)

target_link_libraries(test_memory PRIVATE
  cmocka
)

add_test(NAME test_memory COMMAND ./test_memory)
