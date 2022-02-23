add_link_options(-static -nostdlib)
add_executable(app-wip test.c)
add_dependencies(app-wip sdk ux)
# XXX: order of target link libraries seems important to avoid the following
# error:
# bagl.c:(.text+0xcc): undefined reference to `bagl_draw_with_context'
target_link_libraries(app-wip PRIVATE ux sdk)
