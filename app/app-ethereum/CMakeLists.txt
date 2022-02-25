include_directories(protobuf/ /usr/local/include/ ../nanopb)
link_directories(/usr/local/lib)

add_executable(app-ethereum
               src/app-ethereum.c
               src/get_pubkey.c
               src/ui.c
               src/utils.c
               protobuf/message.pb.h
               protobuf/message.pb.c
               ${NANOPB_PATH}/pb_decode.c
               ${NANOPB_PATH}/pb_encode.c
               ${NANOPB_PATH}/pb_common.c
               )
add_dependencies(app-ethereum
                 sdk
                 ux)
target_link_libraries(app-ethereum PRIVATE
                      sdk
                      ux)
