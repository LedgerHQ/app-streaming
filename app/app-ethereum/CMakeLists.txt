include_directories(protobuf/ ${JSMN_PATH} ${NANOPB_PATH})

add_executable(app-ethereum
  src/app-ethereum.c
  src/eip712.c
  src/get_pubkey.c
  src/rlp.c
  src/sign.c
  src/sign_eip712.c
  src/sign_message.c
  src/sign_tx.c
  src/ui.c
  src/utils.c
  src/version.c
  protobuf/message.pb.h
  protobuf/message.pb.c
  ${NANOPB_PATH}/pb_decode.c
  ${NANOPB_PATH}/pb_encode.c
  ${NANOPB_PATH}/pb_common.c
  )

add_dependencies(app-ethereum
  eip712
  sdk
  ux)

target_link_libraries(app-ethereum PRIVATE
  eip712
  ux
  sdk
  c)

add_subdirectory(src/eip712)

if (NATIVE)
  add_subdirectory(tests)
endif()
