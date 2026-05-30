cmake_minimum_required(VERSION 3.16)

include(cmake/version.cmake)
project(TessesCrossLang VERSION ${CROSSLANG_MAJOR_VERSION}.${CROSSLANG_MINOR_VERSION}.${CROSSLANG_PATCH_VERSION})

set(CMAKE_CXX_STANDARD 17)
include(CMakePackageConfigHelpers)
include(GNUInstallDirs)
include(CheckLibraryExists)
include(FetchContent)
include(cmake/sources.cmake)
include(cmake/options.cmake)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
if(CROSSLANG_ENABLE_FFI)
find_package(PkgConfig)
endif()



include(cmake/findtf.cmake)
include(cmake/sources.cmake)
include(cmake/cpack.cmake)
if(CROSSLANG_STATIC)
include(cmake/staticlib.cmake)
else()
include(cmake/sharedlib.cmake)
endif()

include(cmake/install-dev.cmake)

include(cmake/app.cmake)

if(CROSSLANG_INSTALL_DEVELOPMENT)
add_subdirectory(pkgconfig)
endif()