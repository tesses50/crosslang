cmake_minimum_required(VERSION 3.16)

include(cmake/version.cmake)
include(cmake/options.cmake)

project(TessesCrossLang VERSION ${CROSSLANG_MAJOR_VERSION}.${CROSSLANG_MINOR_VERSION}.${CROSSLANG_PATCH_VERSION})

set(CMAKE_CXX_STANDARD 17)


include(CMakePackageConfigHelpers)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)


if(CROSSLANG_CUSTOM_CONSOLE)
set(CROSSLANG_ENABLE_BINARIES OFF)
set(CROSSLANG_ENABLE_SHARED OFF)
endif()


if(CROSSLANG_FETCHCONTENT)

set(TESSESFRAMEWORK_ENABLE_EXAMPLES OFF)
set(TESSESFRAMEWORK_ENABLE_APPS OFF)

if(NOT TESSESFRAMEWORK_ENABLE_SHARED)
set(CROSSLANG_ENABLE_SHARED OFF)
endif()
if(NOT TESSESFRAMEWORK_ENABLE_STATIC)
set(CROSSLANG_ENABLE_STATIC OFF)
endif()
if(NOT TESSESFRAMEWORK_ENABLE_RPATH)
set(CROSSLANG_ENABLE_RPATH OFF)
endif()

#set(CROSSLANG_ENABLE_SHARED OFF)
#set(TESSESFRAMEWORK_ENABLE_SHARED OFF)
#set(CROSSLANG_SHARED_EXECUTABLES OFF)
include(FetchContent)
FetchContent_Declare(
    TessesFramework
    GIT_REPOSITORY https://git.tesses.org/tesses50/tessesframework.git
)
FetchContent_MakeAvailable(TessesFramework)
list(APPEND TessesCrossLangLibs ${TessesFrameworkTargets})
else()
find_package(TessesFramework REQUIRED)
endif()

if(CROSSLANG_ENABLE_FFI)
find_package(PkgConfig)
endif()
include(cmake/linkdeps.cmake)
if(CROSSLANG_ENABLE_FFI AND CROSSLANG_ENABLE_SHARED)
pkg_check_modules(LIBFFI REQUIRED IMPORTED_TARGET libffi)
endif()
configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/CrossLangVersion.h.in "${CMAKE_CURRENT_BINARY_DIR}/include/CrossLangVersion.h"
INSTALL_DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/CrossLangVersion.h)
include(cmake/sources.cmake)

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
CROSSLANG_LINK_DEPS(crosslang_static)
if(CROSSLANG_FETCHCONTENT)

target_link_libraries(crosslang_static PUBLIC tessesframework)
else()
target_link_libraries(crosslang_static PUBLIC TessesFramework::tessesframework)
endif()

list(APPEND TessesCrossLangLibs crosslang_static)
endif()


if(CROSSLANG_ENABLE_SHARED)
add_library(crosslang_shared SHARED ${CROSSLANG_SOURCE})
CROSSLANG_LINK_DEPS(crosslang_shared)
if(CROSSLANG_FETCHCONTENT)
target_link_libraries(crosslang_shared PUBLIC tessesframework_shared)
else()
target_link_libraries(crosslang_shared PUBLIC TessesFramework::tessesframework_shared)
endif()
list(APPEND TessesCrossLangLibs crosslang_shared)
endif()


if(CROSSLANG_ENABLE_SUPERSLIM)
include(cmake/slim.cmake)
else()
include(cmake/shared.cmake)



if(CROSSLANG_ENABLE_BINARIES)
if(CROSSLANG_ENABLE_SHARED AND CROSSLANG_SHARED_EXECUTABLES)
include(cmake/shared_exec.cmake)
elseif(CROSSLANG_ENABLE_STATIC)
include(cmake/static_exec.cmake)
else()
include(cmake/exec.cmake)
endif()

install(TARGETS crossc DESTINATION "${CMAKE_INSTALL_BINDIR}")
install(TARGETS crossvm DESTINATION "${CMAKE_INSTALL_BINDIR}")
install(TARGETS crossint DESTINATION "${CMAKE_INSTALL_BINDIR}")
install(TARGETS crossdump DESTINATION "${CMAKE_INSTALL_BINDIR}")
install(TARGETS crosslang DESTINATION "${CMAKE_INSTALL_BINDIR}")
install(TARGETS crossarchiveextract DESTINATION "${CMAKE_INSTALL_BINDIR}")
install(TARGETS crossarchivecreate DESTINATION "${CMAKE_INSTALL_BINDIR}")
install(TARGETS crossasm DESTINATION "${CMAKE_INSTALL_BINDIR}")
install(TARGETS crossdisasm DESTINATION "${CMAKE_INSTALL_BINDIR}")
install(TARGETS crossmerge DESTINATION "${CMAKE_INSTALL_BINDIR}")
if(NOT WIN32)
install(TARGETS crossthumbnailer DESTINATION "${CMAKE_INSTALL_BINDIR}")

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
endif()
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
set(CPACK_PACKAGE_VENDOR "Tesses")
set(CPACK_NSIS_CREATE_ICONS_EXTRA "CreateShortCut '$DESKTOP\\\\CrossLang Interperter.lnk' '$INSTDIR\\\\bin\\\\crossint.exe'")
set(CPACK_NSIS_CREATE_ICONS_EXTRA "CreateShortCut '$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\CrossLang Interperter.lnk' '$INSTDIR\\\\bin\\\\crossint.exe'")


include(CPack)
if(CROSSLANG_INSTALL_DEVELOPMENT)
add_subdirectory(pkgconfig)
endif()
