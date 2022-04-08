cmake_minimum_required(VERSION 3.10)

project(RISC-V-apps C ASM)

include(CTest)

option(NATIVE "Build for the host machine instead of RISC-V" OFF)

#set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)

if (NOT NATIVE)
  if (0)
    message(STATUS "Using clang compiler")
    set(CMAKE_C_COMPILER /usr/bin/clang)
    set(CMAKE_ASM_COMPILER /usr/bin/clang)
    add_compile_options(--target=riscv32 -Oz)
    add_compile_options(-march=rv32im)
    # use gcc linker
    add_link_options(-fuse-ld=/usr/xcc/riscv32-unknown-linux-gnu/bin/riscv32-unknown-linux-gnu-ld --target=riscv32)
    add_link_options(-L/usr/xcc/riscv32-unknown-linux-gnu/lib/gcc/riscv32-unknown-linux-gnu/11.2.0/)
  else ()
    set(CMAKE_C_COMPILER /usr/xcc/riscv32-unknown-linux-gnu/bin/riscv32-unknown-linux-gnu-gcc)
    add_compile_options(-mno-div -mno-fdiv -mstrict-align -Os)
    add_compile_options(-march=rv32g -mabi=ilp32)
    message(STATUS "Using default compiler ${CMAKE_C_COMPILER}")
  endif ()

  add_link_options(-z noseparate-code -static -nostdlib)

  include_directories(/usr/local/riscv32-unknown-linux-gnu/include/ sdk/risc-v/)
  link_directories(/usr/local/riscv32-unknown-linux-gnu/lib)
else()
  message(STATUS "Building native binaries")

  if(NOT DEFINED ENV{SPECULOS_DIR})
    message(FATAL_ERROR "Please set SPECULOS_DIR environment variable")
  endif()

  if(NOT DEFINED ENV{BOLOS_SDK_DIR})
    message(FATAL_ERROR "Please set BOLOS_SDK_DIR environment variable")
  endif()

  set(SPECULOS_DIR "$ENV{SPECULOS_DIR}" CACHE INTERNAL "Copied from environment variable")
  set(BOLOS_SDK_DIR "$ENV{BOLOS_SDK_DIR}" CACHE INTERNAL "Copied from environment variable")

  include_directories(sdk/native/ ${BOLOS_SDK_DIR}/include/ ${BOLOS_SDK_DIR}/lib_cxng/include/)

  add_compile_definitions (cxng PUBLIC HAVE_NES_CRYPT HAVE_ST_EDES_PLUS HAVE_ST_AES HAVE_CRC HAVE_HASH HAVE_RIPEMD160 HAVE_SHA224 HAVE_SHA256 HAVE_SHA3 HAVE_SHA384 HAVE_SHA512 HAVE_SHA512_WITH_BLOCK_ALT_METHOD HAVE_SHA512_WITH_BLOCK_ALT_METHOD_M0 HAVE_BLAKE2 HAVE_GROESTL HAVE_HMAC HAVE_PBKDF2 HAVE_DES HAVE_AES HAVE_MATH HAVE_RNG HAVE_RNG_RFC6979 HAVE_RNG_SP800_90A HAVE_ECC HAVE_ECC_WEIERSTRASS HAVE_ECC_TWISTED_EDWARDS HAVE_ECC_MONTGOMERY HAVE_SECP256K1_CURVE HAVE_SECP256R1_CURVE HAVE_SECP384R1_CURVE HAVE_SECP521R1_CURVE HAVE_FR256V1_CURVE HAVE_STARK256_CURVE HAVE_BRAINPOOL_P256R1_CURVE HAVE_BRAINPOOL_P256T1_CURVE HAVE_BRAINPOOL_P320R1_CURVE HAVE_BRAINPOOL_P320T1_CURVE HAVE_BRAINPOOL_P384R1_CURVE HAVE_BRAINPOOL_P384T1_CURVE HAVE_BRAINPOOL_P512R1_CURVE HAVE_BRAINPOOL_P512T1_CURVE HAVE_BLS12_381_G1_CURVE HAVE_CV25519_CURVE HAVE_CV448_CURVE HAVE_ED25519_CURVE HAVE_ED448_CURVE HAVE_ECDH HAVE_ECDSA HAVE_EDDSA HAVE_ECSCHNORR HAVE_X25519 HAVE_X448)

  add_compile_definitions(NATIVE)
  add_compile_options(-fsanitize=address -g -ggdb)
  add_link_options(-fsanitize=address -g -ggdb)
  link_libraries(crypto)

  enable_testing()
endif()

add_compile_options(-Werror)

# Compute the absolute path of jsmn/ and nanopb/ and store them in the variables JSMN_PATH and NANOPB_PATH
if (${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.19.0")
  file(REAL_PATH nanopb/ NANOPB_PATH)
  file(REAL_PATH jsmn/ JSMN_PATH)
else()
  get_filename_component(NANOPB_PATH nanopb/ REALPATH)
  get_filename_component(JSMN_PATH jsmn/ REALPATH)
endif()

# Make sure to include sdk/ first to prevent ${SPECULOS_DIR}/src/sdk.h
# being included instead of sdk/sdk.h.
include_directories(BEFORE ./ sdk/)

add_subdirectory(sdk)
add_subdirectory(ux)

add_subdirectory(app-ethereum)
add_subdirectory(app-sha256)
add_subdirectory(app-wip)
