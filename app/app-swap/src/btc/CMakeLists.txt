include_directories(../)

add_library(swap-btc
  base58.c
  bcd.c
  btc.c
  btchip_helpers.c
  segwit.c)
