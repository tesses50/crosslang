set(PKGCONFIG_INCLUDEDIR "\${prefix}/${CMAKE_INSTALL_INCLUDEDIR}")
set(PKGCONFIG_LIBDIR "\${prefix}/${CMAKE_INSTALL_LIBDIR}")

set(PKGCONFIG_PROJECT_DESCRIPTION "A programming language")
set(PKGCONFIG_PROJECT_HOMEPAGE_URL "https://crosslang.tesseslanguage.com/")

if(CROSSLANG_ENABLE_FFI)
set(PKGCONFIG_DEPS "Requires: libffi")
else()
set(PKGCONFIG_DEPS "")
endif()


configure_file(crosslang.pc.in crosslang.pc @ONLY)
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/crosslang.pc
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
