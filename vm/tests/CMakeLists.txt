cmake_minimum_required(VERSION 3.10)

project(VMtests)

include(CTest)

add_executable(test_memory
  ../src/lfsr.c
  mock_stream.c
  test_memory.c
)

target_compile_options(test_memory PRIVATE -Werror -DNATIVE)

target_link_libraries(test_memory PRIVATE
  cmocka
)

add_test(NAME test_memory COMMAND ./test_memory)
