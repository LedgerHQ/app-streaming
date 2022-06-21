if (NOT NATIVE)
  add_library(sdk
    crypto.c
    risc-v/div.S
    risc-v/ecall-riscv.c
    risc-v/math.c
    risc-v/start.S
    risc-v/std.c
    sdk.c
    uint256.c)

  add_subdirectory(ux)
else()
  add_compile_options(-DNATIVE_64BITS -DOS_LITTLE_ENDIAN)

  add_library(speculos
    ${SPECULOS_DIR}/src/bolos/bagl.c
    ${SPECULOS_DIR}/src/bolos/cx_aes.c
    ${SPECULOS_DIR}/src/bolos/cx_aes_sdk2.c
    ${SPECULOS_DIR}/src/bolos/cx_blake2b.c
    ${SPECULOS_DIR}/src/bolos/cx_bn.c
    ${SPECULOS_DIR}/src/bolos/cx.c
    ${SPECULOS_DIR}/src/bolos/cx_crc.c
    ${SPECULOS_DIR}/src/bolos/cx_curve25519.c
    ${SPECULOS_DIR}/src/bolos/cx_ec.c
    ${SPECULOS_DIR}/src/bolos/cx_ec_domain.c
    ${SPECULOS_DIR}/src/bolos/cx_ecpoint.c
    ${SPECULOS_DIR}/src/bolos/cx_ed25519.c
    ${SPECULOS_DIR}/src/bolos/cx_hash.c
    ${SPECULOS_DIR}/src/bolos/cx_hkdf.c
    ${SPECULOS_DIR}/src/bolos/cx_hmac.c
    ${SPECULOS_DIR}/src/bolos/cxlib.c
    ${SPECULOS_DIR}/src/bolos/cx_math.c
    ${SPECULOS_DIR}/src/bolos/cx_montgomery.c
    ${SPECULOS_DIR}/src/bolos/cx_mpi.c
    ${SPECULOS_DIR}/src/bolos/cx_ripemd160.c
    ${SPECULOS_DIR}/src/bolos/cx_rng_rfc6979.c
    ${SPECULOS_DIR}/src/bolos/cx_scc.c
    ${SPECULOS_DIR}/src/bolos/cx_sha256.c
    ${SPECULOS_DIR}/src/bolos/cx_sha3.c
    ${SPECULOS_DIR}/src/bolos/cx_sha512.c
    ${SPECULOS_DIR}/src/bolos/cx_twisted_edwards.c
    ${SPECULOS_DIR}/src/bolos/cx_utils.c
    ${SPECULOS_DIR}/src/bolos/cx_weierstrass.c
    ${SPECULOS_DIR}/src/bolos/default.c
    ${SPECULOS_DIR}/src/bolos/endorsement.c
    ${SPECULOS_DIR}/src/bolos/os_1.2.c
    ${SPECULOS_DIR}/src/bolos/os_1.5.c
    ${SPECULOS_DIR}/src/bolos/os_1.6.c
    ${SPECULOS_DIR}/src/bolos/os_2.0.c
    ${SPECULOS_DIR}/src/bolos/os_bip32.c
    ${SPECULOS_DIR}/src/bolos/os.c
    ${SPECULOS_DIR}/src/bolos/os_eip2333.c
    ${SPECULOS_DIR}/src/bolos/seproxyhal.c

    native/speculos.c
    )

  target_include_directories (speculos PUBLIC ${SPECULOS_DIR}/src/ ${SPECULOS_DIR}/src/bolos/)

  add_library(cxng
    ${BOLOS_SDK_DIR}/lib_cxng/src/cx_ades.c
    ${BOLOS_SDK_DIR}/lib_cxng/src/cx_blake2b.c
    ${BOLOS_SDK_DIR}/lib_cxng/src/cx_crc16.c
    ${BOLOS_SDK_DIR}/lib_cxng/src/cx_crc32.c
    ${BOLOS_SDK_DIR}/lib_cxng/src/cx_ecdh.c
    ${BOLOS_SDK_DIR}/lib_cxng/src/cx_ecdsa.c
    ${BOLOS_SDK_DIR}/lib_cxng/src/cx_ecfp.c
    ${BOLOS_SDK_DIR}/lib_cxng/src/cx_ecschnorr.c
    ${BOLOS_SDK_DIR}/lib_cxng/src/cx_eddsa.c
    ${BOLOS_SDK_DIR}/lib_cxng/src/cx_exported_functions.c
    ${BOLOS_SDK_DIR}/lib_cxng/src/cx_Groestl-ref.c
    ${BOLOS_SDK_DIR}/lib_cxng/src/cx_hash.c
    ${BOLOS_SDK_DIR}/lib_cxng/src/cx_hkdf.c
    ${BOLOS_SDK_DIR}/lib_cxng/src/cx_hmac.c
    ${BOLOS_SDK_DIR}/lib_cxng/src/cx_init.c
    ${BOLOS_SDK_DIR}/lib_cxng/src/cx_math.c
    ${BOLOS_SDK_DIR}/lib_cxng/src/cx_pbkdf2.c
    ${BOLOS_SDK_DIR}/lib_cxng/src/cx_pkcs1.c
    ${BOLOS_SDK_DIR}/lib_cxng/src/cx_ram.c
    ${BOLOS_SDK_DIR}/lib_cxng/src/cx_ripemd160.c
    ${BOLOS_SDK_DIR}/lib_cxng/src/cx_rng.c
    ${BOLOS_SDK_DIR}/lib_cxng/src/cx_rng_rfc6979.c
    ${BOLOS_SDK_DIR}/lib_cxng/src/cx_rsa.c
    ${BOLOS_SDK_DIR}/lib_cxng/src/cx_selftests.c
    ${BOLOS_SDK_DIR}/lib_cxng/src/cx_sha256.c
    ${BOLOS_SDK_DIR}/lib_cxng/src/cx_sha3.c
    ${BOLOS_SDK_DIR}/lib_cxng/src/cx_sha512.c
    ${BOLOS_SDK_DIR}/lib_cxng/src/cx_utils.c
    )

  target_include_directories (cxng PUBLIC ${BOLOS_SDK_DIR}/include ${BOLOS_SDK_DIR}/lib_cxng/include)

  add_library(sdk
    crypto.c
    native/ecall-native.c
    native/ecall-vm.c
    sdk.c
    uint256.c
    ../../vm/src/ecall_bolos.c
    ../../vm/src/ecall_hash.c
    ../../vm/src/uint256.c
    )

  add_subdirectory(ux)

  target_include_directories (sdk PUBLIC ${BOLOS_SDK_DIR}/include ${BOLOS_SDK_DIR}/lib_cxng/include)
endif()
