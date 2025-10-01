cmake_minimum_required(VERSION 3.16)

project(CrossLangWii)


set(TESSESFRAMEWORK_ENABLE_RPATH OFF CACHE INTERNAL "For TessesFramework" FORCE)
set(TESSESFRAMEWORK_ENABLE_STATIC ON CACHE INTERNAL "For TessesFramework" FORCE)
set(TESSESFRAMEWORK_ENABLE_EXAMPLES OFF CACHE INTERNAL "For TessesFramework" FORCE)
set(TESSESFRAMEWORK_ENABLE_APPS OFF CACHE INTERNAL "For TessesFramework" FORCE)
set(TESSESFRAMEWORK_ENABLE_SHARED OFF CACHE INTERNAL "For TessesFramework" FORCE)
set(CROSSLANG_ENABLE_BINARIES OFF CACHE INTERNAL "For CrossLang" FORCE)
set(MBEDTLS_DIR "/opt/devkitpro/portlibs/wii" CACHE INTERNAL "For TessesFramework" FORCE)

include(FetchContent)

FetchContent_Declare(
    TessesCrossLang
    GIT_REPOSITORY https://onedev.site.tesses.net/crosslang.git
)
 
FetchContent_MakeAvailable(TessesCrossLang)



add_executable(boot main.cpp)
target_link_libraries(boot PUBLIC crosslang_static)
