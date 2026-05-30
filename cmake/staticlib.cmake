
add_library(crosslang STATIC ${CROSSLANG_SOURCE})

include(${CMAKE_CURRENT_LIST_DIR}/helpers.cmake)

list(APPEND CrossLangLibs crosslang)