include_directories(../src/)

add_executable(test_swap
  test_swap.c
  ../src/currency_config.c
  ../src/partner.c
  ../src/swap.c
  ../protobuf/message.pb.h
  ../protobuf/message.pb.c
  ../protobuf/swap.pb.h
  ../protobuf/swap.pb.c
  ${NANOPB_PATH}/pb_decode.c
  ${NANOPB_PATH}/pb_encode.c
  ${NANOPB_PATH}/pb_common.c)

target_compile_options(test_swap PRIVATE -DTEST_PUBLIC_KEY)

add_dependencies(test_swap
  swap-btc
  swap-eth
  sdk)

target_link_libraries(test_swap PRIVATE
  cmocka
  sdk
  swap-btc
  swap-eth
  cxng
  speculos)

add_executable(test_eth_tx
  test_eth_tx.c
)

add_dependencies(test_eth_tx
  swap-eth
  sdk)

target_link_libraries(test_eth_tx PRIVATE
  cmocka
  sdk
  swap-eth
  cxng
  speculos)

add_test(NAME test_eth_tx COMMAND ./test_eth_tx)
add_test(NAME test_swap COMMAND ./test_swap)
