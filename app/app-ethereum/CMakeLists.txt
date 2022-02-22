include_directories(protobuf/ /usr/local/include/)
link_directories(/usr/local/lib)

add_executable(app-ethereum
               src/app-ethereum.c
               src/get_pubkey.c
               src/sign_tx.c
               src/ui.c
               src/utils.c
               protobuf/message.pb-c.h
               protobuf/message.pb-c.c)
add_dependencies(app-ethereum
                 sdk
                 ux)
target_link_libraries(app-ethereum PRIVATE
                      sdk
                      ux
                      protobuf-c)
