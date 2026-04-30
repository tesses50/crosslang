option(CROSSLANG_ENABLE_STATIC "Enable Tesses CrossLang static libraries" ON)
option(CROSSLANG_ENABLE_SHARED "Enable Tesses CrossLang shared libraries" ON)
option(CROSSLANG_ENABLE_BINARIES "Enable Tesses CrossLang binaries" ON)
option(CROSSLANG_INSTALL_DEVELOPMENT "Enable Tesses CrossLang development files" ON)
option(CROSSLANG_ENABLE_THREADING "Enable Tesses CrossLang threading" ON)
option(CROSSLANG_ENABLE_TERMIOS "Enable termios (For changing terminal options)" ON)
option(CROSSLANG_SHARED_EXECUTABLES "Link with libcrosslang_shared" ON)
option(CROSSLANG_FETCHCONTENT "Use fetchcontent" ON)
option(CROSSLANG_ENABLE_CONFIG_ENVVAR "Allow setting config directory via the environment variable CROSSLANG_CONFIG" ON)
option(CROSSLANG_ENABLE_FFI "Enable libffi" OFF)
option(CROSSLANG_ENABLE_RPATH "Enable RPATH" ON)
option(CROSSLANG_ENABLE_SUPERSLIM "Enable crosslang superslim, one binary" OFF)

option(CROSSLANG_CUSTOM_CONSOLE "Enable custom Console" OFF)



set(CROSSLANG_OFFLINE_SHELL_PACKAGE "" CACHE FILEPATH "Path to the shell package generated from https://git.tesses.org/crosslangextras")
