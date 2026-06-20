if(CROSSLANG_FETCHCONTENT)


set(CROSSLANG_STATIC ${TESSESFRAMEWORK_STATIC} CACHE INTERNAL "For CrossLang" FORCE)
FetchContent_Declare(
    TessesFramework
    GIT_REPOSITORY https://git.tesses.org/tesses50/tessesframework.git
    GIT_TAG bc5821cbf0ce055a0d7625039abbfb53459426df
)
set(TESSESFRAMEWORK_ENABLE_EXAMPLES OFF)
FetchContent_MakeAvailable(TessesFramework)
else()
find_package(TessesFramework REQUIRED)
endif()
