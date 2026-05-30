if(CROSSLANG_ENABLE_FFI)
if(NOT CROSSLANG_STATIC)
pkg_check_modules(LIBFFI REQUIRED IMPORTED_TARGET libffi)
target_compile_definitions(crosslang PUBLIC CROSSLANG_ENABLE_FFI)
target_link_libraries(crosslang PUBLIC PkgConfig::LIBFFI)

endif()
endif()

if(CROSSLANG_CUSTOM_CONSOLE)
target_compile_definitions(crosslang PRIVATE CROSSLANG_CUSTOM_CONSOLE)

endif()

if(CROSSLANG_STATIC)
target_compile_definitions(crosslang PRIVATE CROSSLANG_STATIC)

endif()

configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/CrossLangVersion.h.in "${CMAKE_CURRENT_BINARY_DIR}/include/CrossLangVersion.h"
INSTALL_DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/CrossLangVersion.h)

target_include_directories(crosslang
    PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)
target_include_directories(crosslang
    PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)

if(TESSESFRAMEWORK_FETCHCONTENT)
target_link_libraries(crosslang PUBLIC tessesframework)
else()
target_link_libraries(crosslang PUBLIC TessesFramework::tessesframework)
endif()