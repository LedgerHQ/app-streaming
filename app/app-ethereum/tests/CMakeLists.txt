include_directories(../src/)

add_executable(test_pubkey
  test_pubkey.c
  ../src/utils.c)

add_dependencies(test_pubkey
                 sdk)
target_link_libraries(test_pubkey PRIVATE
                 cmocka
                 sdk)

add_test(NAME get_pubkey COMMAND ./test_pubkey)
