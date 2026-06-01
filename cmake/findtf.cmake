if(CROSSLANG_FETCHCONTENT)


set(CROSSLANG_STATIC ${TESSESFRAMEWORK_STATIC} CACHE INTERNAL "For CrossLang" FORCE)
FetchContent_Declare(
    TessesFramework
    GIT_REPOSITORY https://git.tesses.org/tesses50/tessesframework.git
    GIT_TAG 41d503cfb535eca95068b0265418ab2f580264d6
)
set(TESSESFRAMEWORK_ENABLE_EXAMPLES OFF)
FetchContent_MakeAvailable(TessesFramework)
else()
find_package(TessesFramework REQUIRED)
endif()