if(CROSSLANG_FETCHCONTENT)


set(CROSSLANG_STATIC ${TESSESFRAMEWORK_STATIC} CACHE INTERNAL "For CrossLang" FORCE)
FetchContent_Declare(
    TessesFramework
    GIT_REPOSITORY https://git.tesses.org/tesses50/tessesframework.git
    GIT_TAG 5b54af1728907559999e446e56497055297a3696
)
set(TESSESFRAMEWORK_ENABLE_EXAMPLES OFF)
FetchContent_MakeAvailable(TessesFramework)
else()
find_package(TessesFramework REQUIRED)
endif()
