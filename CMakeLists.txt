cmake_minimum_required(VERSION 3.20)

project(TessesCrossLang VERSION 1.0)

set(CMAKE_CXX_STANDARD 20)

set(ENABLE_TESTING OFF)
option(CROSSLANG_ENABLE_STATIC "Enable Tesses CrossLang static libraries" ON)
option(CROSSLANG_ENABLE_SHARED "Enable Tesses CrossLang shared libraries" ON)
option(CROSSLANG_ENABLE_BINARIES "Enable Tesses CrossLang binaries" ON)
option(CROSSLANG_INSTALL_DEVELOPMENT "Enable Tesses CrossLang development files" ON)
option(CROSSLANG_ENABLE_THREADING "Enable Tesses CrossLang threading" ON)
option(CROSSLANG_ENABLE_SQLITE "Enable sqlite (Embedded database, supports Wii)" ON)
option(CROSSLANG_ENABLE_JSON "Enable JSON" ON)
option(CROSSLANG_ENABLE_PROCESS "Enable process" ON)
option(CROSSLANG_ENABLE_SDL2 "Enable SDL2 (For Drawing)" ON)
option(CROSSLANG_ENABLE_TERMIOS "Enable termios (For changing terminal options)" ON)



set(JANSSON_DIR "" CACHE PATH "Directory for Jansson")

find_package(TessesFramework REQUIRED)
function(CROSSLANG_LINK_DEPS CROSSLANG_TARGET_NAME)
if(CROSSLANG_ENABLE_PROCESS)
target_compile_definitions(${CROSSLANG_TARGET_NAME} PUBLIC CROSSLANG_ENABLE_PROCESS)
endif()
if(CROSSLANG_ENABLE_JSON)
target_compile_definitions(${CROSSLANG_TARGET_NAME} PUBLIC CROSSLANG_ENABLE_JSON)
if(JANSSON_DIR STREQUAL "")
else()
target_include_directories(${CROSSLANG_TARGET_NAME} PUBLIC ${JANSSON_DIR}/include)
target_link_directories(${CROSSLANG_TARGET_NAME} PUBLIC ${JANSSON_DIR}/lib)
endif()
target_link_libraries(${CROSSLANG_TARGET_NAME} PUBLIC jansson)
endif()
if(CROSSLANG_ENABLE_THREADING)
target_compile_definitions(${CROSSLANG_TARGET_NAME} PUBLIC CROSSLANG_ENABLE_THREADING)
endif()
if(CROSSLANG_ENABLE_TERMIOS)
target_compile_definitions(${CROSSLANG_TARGET_NAME} PUBLIC CROSSLANG_ENABLE_TERMIOS)
endif()

if(CROSSLANG_ENABLE_SQLITE)
target_compile_definitions(${CROSSLANG_TARGET_NAME} PUBLIC CROSSLANG_ENABLE_SQLITE)
endif()


if(CROSSLANG_ENABLE_SDL2)
target_compile_definitions(${CROSSLANG_TARGET_NAME} PUBLIC CROSSLANG_ENABLE_SDL2)
find_package(SDL2 REQUIRED)
find_package(SDL2_ttf REQUIRED)


target_include_directories(${CROSSLANG_TARGET_NAME} PUBLIC ${SDL2_INCLUDE_DIRS})
target_link_libraries(${CROSSLANG_TARGET_NAME} PUBLIC ${SDL2_LIBRARIES})
if("${CMAKE_SYSTEM_NAME}" STREQUAL "NintendoWii" OR "${CMAKE_SYSTEM_NAME}" STREQUAL "NintendoGameCube")
target_link_libraries(${CROSSLANG_TARGET_NAME} PUBLIC SDL2_image png jpeg z)
else()
find_package(SDL2_image REQUIRED)
target_link_libraries(${CROSSLANG_TARGET_NAME} PUBLIC ${SDL2_image_LIBRARIES})
endif()
target_link_libraries(${CROSSLANG_TARGET_NAME} PUBLIC ${SDL2_ttf_LIBRARIES})
endif()
if("${CMAKE_SYSTEM_NAME}" STREQUAL "NintendoWii" OR "${CMAKE_SYSTEM_NAME}" STREQUAL "NintendoGameCube")
target_link_libraries(${CROSSLANG_TARGET_NAME} PUBLIC fat)
endif()
if("${CMAKE_SYSTEM_NAME}" STREQUAL "NintendoWii")
target_link_libraries(${CROSSLANG_TARGET_NAME} PUBLIC wiisocket)
endif()
target_include_directories(${CROSSLANG_TARGET_NAME}
    PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)
endfunction()

list(APPEND CROSSLANG_SOURCE 
src/compiler/codegen.cpp
src/compiler/lexer.cpp
src/compiler/parser.cpp
src/compiler/ast.cpp
src/runtime_methods/sdl2.cpp
src/runtime_methods/console.cpp
src/runtime_methods/io.cpp
src/runtime_methods/std.cpp
src/runtime_methods/net.cpp
src/runtime_methods/vm.cpp
src/runtime_methods/json.cpp
src/runtime_methods/sqlite.cpp
src/runtime_methods/dictionary.cpp
src/runtime_methods/crypto.cpp
src/runtime_methods/ogc.cpp
src/runtime_methods/path.cpp
src/runtime_methods/env.cpp
src/sago/platform_folders.cpp
src/types/ittr.cpp
src/types/closure.cpp
src/types/dictionary.cpp
src/types/externalmethod.cpp
src/types/list.cpp
src/types/native.cpp
src/types/rootenvironment.cpp
src/types/subenvironment.cpp
src/types/vfsheapobject.cpp
src/types/streamheapobject.cpp
src/vm/filereader.cpp
src/vm/gc.cpp
src/vm/gclist.cpp
src/vm/vm.cpp
src/bitconverter.cpp
)

if(CROSSLANG_ENABLE_SQLITE)
list(APPEND CROSSLANG_SOURCE src/sqlite/sqlite3.c src/sqlite/vfs.c)
endif()



include(GNUInstallDirs)

if(CROSSLANG_ENABLE_STATIC)

add_library(crosslang_static STATIC ${CROSSLANG_SOURCE})
target_link_libraries(crosslang_static PUBLIC TessesFramework::tessesframework)
CROSSLANG_LINK_DEPS(crosslang_static)
list(APPEND TessesCrossLangLibs crosslang_static)
endif()


if(CROSSLANG_ENABLE_SHARED)

add_library(crosslang_shared SHARED ${CROSSLANG_SOURCE})
CROSSLANG_LINK_DEPS(crosslang_shared)
target_link_libraries(crosslang_shared PUBLIC TessesFramework::tessesframework_shared)
list(APPEND TessesCrossLangLibs crosslang_shared)
endif()




if(CROSSLANG_INSTALL_DEVELOPMENT)
install(TARGETS ${TessesCrossLangLibs} 
    EXPORT TessesCrossLangTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(FILES include/CrossLang.hpp DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(EXPORT TessesCrossLangTargets
    FILE TessesCrossLangTargets.cmake
    NAMESPACE TessesCrossLang::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/TessesCrossLang
)


include(CMakePackageConfigHelpers)
configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/Config.cmake.in "${CMAKE_CURRENT_BINARY_DIR}/TessesCrossLangConfig.cmake"
INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/TessesCrossLang)

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/TessesCrossLangConfig.cmake"
DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/TessesCrossLang)
endif()
if(CROSSLANG_ENABLE_BINARIES)
if(CROSSLANG_ENABLE_STATIC)

add_executable(crossc src/crosslangcompiler.cpp)
add_executable(crossvm src/crosslangvm.cpp)
add_executable(crossint src/crosslanginterperter.cpp)
add_executable(crossdump src/crosslangdump.cpp)
add_executable(crosslang src/crosslang.cpp)
target_link_libraries(crossc PUBLIC crosslang_static)
target_link_libraries(crossvm PUBLIC crosslang_static)
target_link_libraries(crossint PUBLIC crosslang_static)
target_link_libraries(crossdump PUBLIC crosslang_static)
target_link_libraries(crosslang PUBLIC crosslang_static)
else()
add_executable(crossc src/crosslangcompiler.cpp ${CROSSLANG_SOURCE})
add_executable(crossvm src/crosslangvm.cpp ${CROSSLANG_SOURCE})
add_executable(crossint src/crosslanginterperter.cpp ${CROSSLANG_SOURCE})
add_executable(crossdump src/crosslangdump.cpp ${CROSSLANG_SOURCE})
add_executable(crosslang src/crosslang.cpp ${CROSSLANG_SOURCE})
CROSSLANG_LINK_DEPS(crossc)
CROSSLANG_LINK_DEPS(crossvm)
CROSSLANG_LINK_DEPS(crossint)
CROSSLANG_LINK_DEPS(crosslang)
CROSSLANG_LINK_DEPS(crossdump)
endif()
install(TARGETS crossc DESTINATION bin)
install(TARGETS crossvm DESTINATION bin)
install(TARGETS crossint DESTINATION bin)
install(TARGETS crossdump DESTINATION bin)
install(TARGETS crosslang DESTINATION bin)
endif()
include(InstallRequiredSystemLibraries)
set(CPACK_PACKAGE_CONTACT "Mike Nolan <tesses@tesses.net>")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE.md")
set(CPACK_PACKAGE_VERSION_MAJOR "${TessesCrossLang_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${TessesCrossLang_VERSION_MINOR}")
include(CPack)