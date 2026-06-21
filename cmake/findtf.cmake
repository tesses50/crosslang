if(CROSSLANG_FETCHCONTENT)


set(CROSSLANG_STATIC ${TESSESFRAMEWORK_STATIC} CACHE INTERNAL "For CrossLang" FORCE)
FetchContent_Declare(
    TessesFramework
    GIT_REPOSITORY https://git.tesses.org/tesses50/tessesframework.git
    GIT_TAG bd0587779e68832cf382fe49b6b2e99474bcf81f
)
set(TESSESFRAMEWORK_ENABLE_EXAMPLES OFF)
FetchContent_MakeAvailable(TessesFramework)
else()
find_package(TessesFramework REQUIRED)
endif()
