add_link_options(-static -nostdlib)
add_executable(app test.c)
add_dependencies(app sdk ux)
target_link_libraries(app PRIVATE sdk ux)
