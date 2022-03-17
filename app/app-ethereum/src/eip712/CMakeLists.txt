include_directories(../)

add_library(eip712
  example_mail.c
  opensea.c)

target_compile_options(eip712 PRIVATE
  -DJSMN_STATIC)
