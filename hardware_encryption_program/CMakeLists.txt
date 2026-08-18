cmake_minimum_required(VERSION 3.16)

project(hardware_encryption_program VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Quick QuickControls2 Widgets)

qt_standard_project_setup(REQUIRES 6.10)

# Content of downloaded design from Figma to Qt or similar
if (EXISTS "${CMAKE_CURRENT_LIST_DIR}/importedcontent/CMakeLists.txt")
    add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/importedcontent)
endif()

qt_add_executable(apphardware_encryption_program
    main.cpp
)

qt_add_qml_module(apphardware_encryption_program
    URI hardware_encryption_program
    QML_FILES
        Main.qml
        SOURCES backend.h backend.cpp
SOURCES crypto/crypto_helper.cpp crypto/crypto_helper.h crypto/crypto_hid_controller.cpp crypto/crypto_hid_controller.h crypto/encrypted_file.cpp crypto/encrypted_file.h crypto/hid.cpp crypto/hid.h sodium/core.h sodium/crypto_aead_aegis128l.h sodium/crypto_aead_aegis256.h sodium/crypto_aead_aes256gcm.h sodium/crypto_aead_chacha20poly1305.h sodium/crypto_aead_xchacha20poly1305.h sodium/crypto_auth.h sodium/crypto_auth_hmacsha256.h sodium/crypto_auth_hmacsha512.h sodium/crypto_auth_hmacsha512256.h sodium/crypto_box.h sodium/crypto_box_curve25519xchacha20poly1305.h sodium/crypto_box_curve25519xsalsa20poly1305.h sodium/crypto_core_ed25519.h sodium/crypto_core_hchacha20.h sodium/crypto_core_hsalsa20.h sodium/crypto_core_keccak1600.h sodium/crypto_core_ristretto255.h sodium/crypto_core_salsa20.h sodium/crypto_core_salsa2012.h sodium/crypto_core_salsa208.h sodium/crypto_generichash.h sodium/crypto_generichash_blake2b.h sodium/crypto_hash.h sodium/crypto_hash_sha256.h sodium/crypto_hash_sha3.h sodium/crypto_hash_sha512.h sodium/crypto_ipcrypt.h sodium/crypto_kdf.h sodium/crypto_kdf_blake2b.h sodium/crypto_kdf_hkdf_sha256.h sodium/crypto_kdf_hkdf_sha512.h sodium/crypto_kem.h sodium/crypto_kem_mlkem768.h sodium/crypto_kem_xwing.h sodium/crypto_kx.h sodium/crypto_onetimeauth.h sodium/crypto_onetimeauth_poly1305.h sodium/crypto_pwhash.h sodium/crypto_pwhash_argon2i.h sodium/crypto_pwhash_argon2id.h sodium/crypto_pwhash_scryptsalsa208sha256.h sodium/crypto_scalarmult.h sodium/crypto_scalarmult_curve25519.h sodium/crypto_scalarmult_ed25519.h sodium/crypto_scalarmult_ristretto255.h sodium/crypto_secretbox.h sodium/crypto_secretbox_xchacha20poly1305.h sodium/crypto_secretbox_xsalsa20poly1305.h sodium/crypto_secretstream_xchacha20poly1305.h sodium/crypto_shorthash.h sodium/crypto_shorthash_siphash24.h sodium/crypto_sign.h sodium/crypto_sign_ed25519.h sodium/crypto_sign_edwards25519sha512batch.h sodium/crypto_stream.h sodium/crypto_stream_chacha20.h sodium/crypto_stream_salsa20.h sodium/crypto_stream_salsa2012.h sodium/crypto_stream_salsa208.h sodium/crypto_stream_xchacha20.h sodium/crypto_stream_xsalsa20.h sodium/crypto_verify_16.h sodium/crypto_verify_32.h sodium/crypto_verify_64.h sodium/crypto_xof_shake128.h sodium/crypto_xof_shake256.h sodium/crypto_xof_turboshake128.h sodium/crypto_xof_turboshake256.h sodium/export.h sodium/randombytes.h sodium/randombytes_internal_random.h sodium/randombytes_sysrandom.h sodium/runtime.h sodium/utils.h sodium/version.h
RESOURCES libsodium.dll libsodium.exp libsodium.lib libsodium.pdb
SOURCES sodium.h)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
set_target_properties(apphardware_encryption_program PROPERTIES
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(apphardware_encryption_program
    PRIVATE Qt6::Quick Qt6::QuickControls2 Qt6::Widgets
    ${CMAKE_CURRENT_SOURCE_DIR}/libsodium.dll
    -lhid -lsetupapi
)

add_custom_command(TARGET apphardware_encryption_program POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_CURRENT_SOURCE_DIR}/libsodium.dll"
    "$<TARGET_FILE_DIR:apphardware_encryption_program>"
)

install(TARGETS apphardware_encryption_program
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

target_include_directories(apphardware_encryption_program
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../shared)