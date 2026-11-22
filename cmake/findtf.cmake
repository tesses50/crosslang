if(CROSSLANG_FETCHCONTENT)


set(CROSSLANG_STATIC ${TESSESFRAMEWORK_STATIC} CACHE INTERNAL "For CrossLang" FORCE)
FetchContent_Declare(
    TessesFramework
    GIT_REPOSITORY https://git.tesses.org/tesses50/tessesframework.git
    GIT_TAG f6f81bf9099e672300a0cd51c3cb0f1c2d1e54e7
)
set(TESSESFRAMEWORK_ENABLE_EXAMPLES OFF)
FetchContent_MakeAvailable(TessesFramework)
else()
find_package(TessesFramework REQUIRED)
endif()
