include_directories(../)

add_library(swap-eth
  eth.c
  sign_tx.c
  tx.c
  utils.c)

add_dependencies(swap-eth
  sdk)

target_link_libraries(swap-eth PRIVATE
  sdk)
