if(CROSSLANG_ENABLE_BINARIES)
if(MINGW)
list(APPEND CROSSLANG_WIN32_EXE_SRC "${CMAKE_CURRENT_SOURCE_DIR}/winicon.rc")
if(MINGW)
    ENABLE_LANGUAGE(RC)
endif(MINGW)
endif()
add_executable(crosslang_app src/programs/slim.cpp "${CROSSLANG_WIN32_EXE_SRC}")
target_link_libraries(crosslang_app PUBLIC crosslang)
set_target_properties(crosslang_app PROPERTIES OUTPUT_NAME crosslang)
install(TARGETS crosslang_app DESTINATION "${CMAKE_INSTALL_BINDIR}")
endif()

if(NOT WIN32 AND NOT APPLE)
configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/crossvm-binfmt.conf.in "${CMAKE_CURRENT_BINARY_DIR}/crossvm-binfmt.conf"
INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/binfmt.d)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/crossvm-binfmt.conf"
DESTINATION ${CMAKE_INSTALL_LIBDIR}/binfmt.d)
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/crosslang.xml"
DESTINATION ${CMAKE_INSTALL_PREFIX}/share/mime/packages)
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/crosslang.png"
DESTINATION ${CMAKE_INSTALL_PREFIX}/share/icons)
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/crosslang.thumbnailer"
DESTINATION ${CMAKE_INSTALL_PREFIX}/share/thumbnailers)
endif()