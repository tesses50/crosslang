cmake_minimum_required(VERSION 3.16)

project(TessesCrossLang VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)


include(CMakePackageConfigHelpers)
option(CROSSLANG_ENABLE_STATIC "Enable Tesses CrossLang static libraries" ON)
option(CROSSLANG_ENABLE_SHARED "Enable Tesses CrossLang shared libraries" ON)
option(CROSSLANG_ENABLE_BINARIES "Enable Tesses CrossLang binaries" ON)
option(CROSSLANG_INSTALL_DEVELOPMENT "Enable Tesses CrossLang development files" ON)
option(CROSSLANG_ENABLE_THREADING "Enable Tesses CrossLang threading" ON)
option(CROSSLANG_ENABLE_SQLITE "Enable sqlite (Embedded database, supports Wii)" ON)
option(CROSSLANG_ENABLE_JSON "Enable JSON" ON)
option(CROSSLANG_ENABLE_PROCESS "Enable process" ON)
option(CROSSLANG_ENABLE_TERMIOS "Enable termios (For changing terminal options)" ON)
option(CROSSLANG_ENABLE_PLATFORM_FOLDERS "Enable platform folders" ON)
option(CROSSLANG_SHARED_EXECUTABLES "Link with libcrosslang_shared" ON)

option(CROSSLANG_CUSTOM_CONSOLE "Enable custom Console" OFF)


if(CROSSLANG_CUSTOM_CONSOLE)
set(CROSSLANG_ENABLE_BINARIES OFF)
set(CROSSLANG_ENABLE_SHARED OFF)
endif()

set(CROSSLANG_OFFLINE_SHELL_PACKAGE "" CACHE FILEPATH "Path to the shell package generated from https://onedev.site.tesses.net/CrossLang/CrossLangExtras")
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

if(CROSSLANG_CUSTOM_CONSOLE)
target_compile_definitions(${CROSSLANG_TARGET_NAME} PUBLIC CROSSLANG_CUSTOM_CONSOLE)
endif()
if(CROSSLANG_ENABLE_PLATFORM_FOLDERS)
target_compile_definitions(${CROSSLANG_TARGET_NAME} PUBLIC CROSSLANG_ENABLE_PLATFORM_FOLDERS)
endif()
if(CROSSLANG_ENABLE_SHARED)
target_compile_definitions(${CROSSLANG_TARGET_NAME} PUBLIC CROSSLANG_ENABLE_SHARED)

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
src/runtime_methods/process.cpp
src/runtime_methods/time.cpp
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
src/archive.cpp
)

if(CROSSLANG_ENABLE_PLATFORM_FOLDERS)
list(APPEND CROSSLANG_SOURCE src/sago/platform_folders.cpp)
endif()
if(CROSSLANG_ENABLE_SQLITE)
list(APPEND CROSSLANG_SOURCE src/sqlite/sqlite3.c src/sqlite/vfs.c)
endif()
if(CROSSLANG_OFFLINE_SHELL_PACKAGE STREQUAL "")

else()
install(FILES ${CROSSLANG_OFFLINE_SHELL_PACKAGE} DESTINATION share/Tesses/CrossLang)
endif()

if(MINGW)
list(APPEND CROSSLANG_WIN32_EXE_SRC "${CMAKE_CURRENT_SOURCE_DIR}/winicon.rc")
if(MINGW)
    ENABLE_LANGUAGE(RC)
endif(MINGW)
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

configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/Config.cmake.in "${CMAKE_CURRENT_BINARY_DIR}/TessesCrossLangConfig.cmake"
INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/TessesCrossLang)

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/TessesCrossLangConfig.cmake"
DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/TessesCrossLang)
else()
if(CROSSLANG_ENABLE_SHARED)
install(TARGETS crosslang_shared
    EXPORT TessesCrossLangTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
endif()
endif()



if(CROSSLANG_ENABLE_BINARIES)
if(CROSSLANG_ENABLE_SHARED AND CROSSLANG_SHARED_EXECUTABLES)
set(CMAKE_MACOSX_RPATH 1)
set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
add_executable(crossc src/crosslangcompiler.cpp ${CROSSLANG_WIN32_EXE_SRC})
add_executable(crossvm src/crosslangvm.cpp ${CROSSLANG_WIN32_EXE_SRC})
add_executable(crossint src/crosslanginterperter.cpp ${CROSSLANG_WIN32_EXE_SRC})
add_executable(crossdump src/crosslangdump.cpp ${CROSSLANG_WIN32_EXE_SRC})
add_executable(crosslang src/crosslang.cpp ${CROSSLANG_WIN32_EXE_SRC})
add_executable(crossarchiveextract src/crossarchiveextract.cpp ${CROSSLANG_WIN32_EXE_SRC})
add_executable(crossarchivecreate src/crossarchivecreate.cpp ${CROSSLANG_WIN32_EXE_SRC})
target_link_libraries(crossc PUBLIC crosslang_shared)
target_link_libraries(crossvm PUBLIC crosslang_shared)
target_link_libraries(crossint PUBLIC crosslang_shared)
target_link_libraries(crossdump PUBLIC crosslang_shared)
target_link_libraries(crosslang PUBLIC crosslang_shared)
target_link_libraries(crossarchiveextract PUBLIC crosslang_shared)
target_link_libraries(crossarchivecreate PUBLIC crosslang_shared)
elseif(CROSSLANG_ENABLE_STATIC)

add_executable(crossc src/crosslangcompiler.cpp ${CROSSLANG_WIN32_EXE_SRC})
add_executable(crossvm src/crosslangvm.cpp ${CROSSLANG_WIN32_EXE_SRC})
add_executable(crossint src/crosslanginterperter.cpp ${CROSSLANG_WIN32_EXE_SRC})
add_executable(crossdump src/crosslangdump.cpp ${CROSSLANG_WIN32_EXE_SRC})
add_executable(crosslang src/crosslang.cpp ${CROSSLANG_WIN32_EXE_SRC})
add_executable(crossarchiveextract src/crossarchiveextract.cpp ${CROSSLANG_WIN32_EXE_SRC})
add_executable(crossarchivecreate src/crossarchivecreate.cpp ${CROSSLANG_WIN32_EXE_SRC})
target_link_libraries(crossc PUBLIC crosslang_static)
target_link_libraries(crossvm PUBLIC crosslang_static)
target_link_libraries(crossint PUBLIC crosslang_static)
target_link_libraries(crossdump PUBLIC crosslang_static)
target_link_libraries(crosslang PUBLIC crosslang_static)
target_link_libraries(crossarchiveextract PUBLIC crosslang_static)
target_link_libraries(crossarchivecreate PUBLIC crosslang_static)
else()
add_executable(crossc src/crosslangcompiler.cpp ${CROSSLANG_SOURCE} ${CROSSLANG_WIN32_EXE_SRC})
add_executable(crossvm src/crosslangvm.cpp ${CROSSLANG_SOURCE} ${CROSSLANG_WIN32_EXE_SRC})
add_executable(crossint src/crosslanginterperter.cpp ${CROSSLANG_SOURCE} ${CROSSLANG_WIN32_EXE_SRC})
add_executable(crossdump src/crosslangdump.cpp ${CROSSLANG_SOURCE} ${CROSSLANG_WIN32_EXE_SRC})
add_executable(crosslang src/crosslang.cpp ${CROSSLANG_SOURCE} ${CROSSLANG_WIN32_EXE_SRC})
add_executable(crossarchiveextract src/crossarchiveextract.cpp ${CROSSLANG_SOURCE} ${CROSSLANG_WIN32_EXE_SRC})
add_executable(crossarchivecreate src/crossarchivecreate.cpp ${CROSSLANG_SOURCE} ${CROSSLANG_WIN32_EXE_SRC})
CROSSLANG_LINK_DEPS(crossc)
CROSSLANG_LINK_DEPS(crossvm)
CROSSLANG_LINK_DEPS(crossint)
CROSSLANG_LINK_DEPS(crosslang)
CROSSLANG_LINK_DEPS(crossdump)
CROSSLANG_LINK_DEPS(crossarchiveextract)
CROSSLANG_LINK_DEPS(crossarchivecreate)
endif()
install(TARGETS crossc DESTINATION bin)
install(TARGETS crossvm DESTINATION bin)
install(TARGETS crossint DESTINATION bin)
install(TARGETS crossdump DESTINATION bin)
install(TARGETS crosslang DESTINATION bin)
install(TARGETS crossarchiveextract DESTINATION bin)
install(TARGETS crossarchivecreate DESTINATION bin)
configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/crossvm-binfmt.conf.in "${CMAKE_CURRENT_BINARY_DIR}/crossvm-binfmt.conf"
INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/binfmt.d)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/crossvm-binfmt.conf"
DESTINATION ${CMAKE_INSTALL_LIBDIR}/binfmt.d)
endif()
include(InstallRequiredSystemLibraries)
set(CPACK_PACKAGE_CONTACT "Mike Nolan <tesses@tesses.net>")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE.md")
set(CPACK_PACKAGE_VERSION_MAJOR "${TessesCrossLang_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${TessesCrossLang_VERSION_MINOR}")

set(CPACK_NSIS_EXTRA_PREINSTALL_COMMANDS
"!include \\\"FileFunc.nsh\\\"\n!include \\\"${CMAKE_CURRENT_SOURCE_DIR}/FileAssociation.nsh\\\"")


# Create association on install
set(CPACK_NSIS_EXTRA_INSTALL_COMMANDS
 "\\\${RegisterExtension} '$INSTDIR\\\\bin\\\\crossvm.exe' '.crvm' 'CrossLang Executable'\n\
 \\\${RefreshShellIcons}")

# Remove association on uninstall


 set(CPACK_NSIS_EXTRA_INSTALL_COMMANDS
 "\\\${RegisterExtension} '$INSTDIR\\\\bin\\\\crossint.exe' '.tcross' 'CrossLang Script'\n\
 \\\${RefreshShellIcons}\n\
 \\\${UnRegisterExtension} '.crvm' 'CrossLang Executable'\n\
 \\\${RefreshShellIcons}")

# Remove association on uninstall
set(CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS
 "\\\${UnRegisterExtension} '.tcross' 'CrossLang Script'\n\
 \\\${RefreshShellIcons}\n\
 \\\${UnRegisterExtension} '.crvm' 'CrossLang Executable'\n\
 \\\${RefreshShellIcons}")

 set(CPACK_NSIS_MUI_ICON "${CMAKE_CURRENT_SOURCE_DIR}/winicon.ico")
set(CPACK_NSIS_MODIFY_PATH ON)
set(CPACK_NSIS_CREATE_ICONS_EXTRA "CreateShortCut '$DESKTOP\\\\CrossLang Interperter.lnk' '$INSTDIR\\\\bin\\\\crossint.exe'")


include(CPack)
