# required for fuzzing
set(CMAKE_C_COMPILER /usr/bin/clang)

include_directories(../src/)

add_executable(test_sign_msg
  test_sign_msg.c
  ../src/get_pubkey.c
  ../src/sign.c
  ../src/sign_message.c
  ../src/ui.c
  ../src/utils.c)

add_dependencies(test_sign_msg
  sdk)

target_link_libraries(test_sign_msg PRIVATE
  cmocka
  ux
  sdk
  cxng
  speculos)

add_executable(test_pubkey
  test_pubkey.c
  ../src/utils.c)

add_dependencies(test_pubkey
  sdk)

target_link_libraries(test_pubkey PRIVATE
  cmocka
  sdk
  cxng
  speculos)

add_executable(test_utils
  test_utils.c
  ../src/utils.c)

add_dependencies(test_utils
  sdk)

target_link_libraries(test_utils PRIVATE
  cmocka
  sdk
  cxng
  speculos)

add_executable(test_eip712
  test_eip712.c
  ../src/ui.c
  ../src/utils.c)

add_dependencies(test_eip712
  eip712
  sdk
  cxng
  speculos
  ux)

target_link_libraries(test_eip712 PRIVATE
  cmocka
  eip712
  ux
  sdk
  cxng
  speculos)

add_executable(fuzz_rlp
  fuzz_rlp.c
  ../src/rlp.c)

target_compile_options(fuzz_rlp PRIVATE -fsanitize=fuzzer,address -g -ggdb2)
target_link_options(fuzz_rlp PRIVATE -fsanitize=fuzzer,address)
target_link_libraries(fuzz_rlp PRIVATE
  sdk
  cxng
  speculos)

add_test(NAME get_pubkey COMMAND ./test_pubkey)
add_test(NAME sign_msg COMMAND ./test_sign_msg)
add_test(NAME test_eip712 COMMAND ./test_eip712)
add_test(NAME utils COMMAND ./test_utils)
