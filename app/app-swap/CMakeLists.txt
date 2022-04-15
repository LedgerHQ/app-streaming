include_directories(protobuf/ ${JSMN_PATH} ${NANOPB_PATH})

add_executable(app-swap
  src/currency_config.c
  src/main.c
  src/partner.c
  src/sell.c
  src/swap.c
  src/ui.c
  src/version.c
  protobuf/message.pb.h
  protobuf/message.pb.c
  protobuf/swap.pb.h
  protobuf/swap.pb.c
  ${NANOPB_PATH}/pb_decode.c
  ${NANOPB_PATH}/pb_encode.c
  ${NANOPB_PATH}/pb_common.c
  )

target_compile_options(app-swap PRIVATE -DTEST_PUBLIC_KEY -DTESTING)

add_dependencies(app-swap
  swap-btc
  swap-eth
  sdk
  ux)

target_link_libraries(app-swap PRIVATE
  swap-btc
  swap-eth
  ux
  sdk
  c)

add_subdirectory(src/btc)
add_subdirectory(src/eth)

if (NATIVE)
  target_link_libraries(app-swap PRIVATE cxng speculos)
  add_subdirectory(tests)
endif()
