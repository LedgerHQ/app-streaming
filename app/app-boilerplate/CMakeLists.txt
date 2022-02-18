# Could NOT find Protobuf
# apt-get install protobuf-compiler libprotobuf-c1

add_link_options(-static -nostdlib)
add_executable(app-boilerplate message.pb-c.h message.pb-c.c app-boilerplate.c sign_tx.c)
add_dependencies(app-boilerplate sdk ux)
target_link_libraries(app-boilerplate PRIVATE sdk ux /app/app-boilerplate/protobuf-c/protobuf-c/.libs/libprotobuf-c.a)

#target_include_directories(app-boilerplate PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/)

include_directories(protobuf-c/)
link_directories(protobuf-c/protobuf-c/.libs/)


#set(Protobuf_LIBRARIES /app/app-boilerplate/protobuf/src})                                
#set(PROTOBUF_INCLUDE_DIRS ${Protobuf_INCLUDE_DIR})  


#set_source_files_properties("message.pb.cc" "message.pb.h" PROPERTIES GENERATED TRUE)

#find_package(Protobuf REQUIRED)
#include_directories(${PROTOBUF_INCLUDE_DIR})
#protobuf_generate_cpp("message.pb.cc" "message.pb.h" message.proto)

#protobuf_generate(
#    LANGUAGE cpp
#    #TARGET "message.pb.cc" "message.pb.h"
#    TARGET app-boilerplate
#    PROTOS message.proto)
#ADD_LIBRARY(proto ${PROTO_HEADER} ${PROTO_SRC})
