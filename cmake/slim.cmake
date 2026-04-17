
add_executable(crosslang WIN32 src/programs/slim.cpp ${CROSSLANG_WIN32_EXE_SRC})

if(CROSSLANG_ENABLE_STATIC)

target_link_libraries(crosslang PUBLIC crosslang_static)
elseif(CROSSLANG_ENABLE_SHARED)
target_link_libraries(crosslang PUBLIC crosslang_shared)
endif()

if(CROSSLANG_ENABLE_SHARED)

install(TARGETS crosslang_shared
    EXPORT TessesCrossLangTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

endif()

install(TARGETS crosslang DESTINATION "${CMAKE_INSTALL_BINDIR}")
