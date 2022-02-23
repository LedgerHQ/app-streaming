add_executable(app-sha256 app-sha256.c sha256sum.c)
add_dependencies(app-sha256 sdk ux)
target_link_libraries(app-sha256 PRIVATE sdk ux)
