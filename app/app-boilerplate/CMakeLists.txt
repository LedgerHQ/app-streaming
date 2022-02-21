add_link_options(-static -nostdlib)
include_directories(protobuf/ /usr/local/include/)
link_directories(/usr/local/lib)

add_executable(app-boilerplate
               src/app-boilerplate.c
               src/sign_tx.c
               protobuf/message.pb-c.h
               protobuf/message.pb-c.c)
add_dependencies(app-boilerplate
                 sdk
                 ux)
target_link_libraries(app-boilerplate PRIVATE
                      sdk
                      ux
                      protobuf-c)
