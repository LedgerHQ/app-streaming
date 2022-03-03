include_directories(../src/)

add_executable(test_pubkey
  test_pubkey.c
  ../src/utils.c)

add_executable(test_utils
  test_utils.c
  ../src/utils.c)

add_dependencies(test_pubkey
  sdk)
add_dependencies(test_utils
                 sdk)
target_link_libraries(test_pubkey PRIVATE
                 cmocka
                 sdk)
target_link_libraries(test_utils PRIVATE
                 cmocka
                 sdk)

add_test(NAME get_pubkey COMMAND ./test_pubkey)
add_test(NAME utils COMMAND ./test_utils)
