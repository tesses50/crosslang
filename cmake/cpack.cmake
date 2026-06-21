include(InstallRequiredSystemLibraries)
set(CPACK_PACKAGE_CONTACT "Mike Nolan <tesses@tesses.net>")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE.md")
set(CPACK_PACKAGE_VERSION_MAJOR "${TessesCrossLang_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${TessesCrossLang_VERSION_MINOR}")

set(CPACK_NSIS_EXTRA_PREINSTALL_COMMANDS
"!include \\\"FileFunc.nsh\\\"\n!include \\\"${CMAKE_CURRENT_SOURCE_DIR}/FileAssociation.nsh\\\"")


# Create association on install
set(CPACK_NSIS_EXTRA_INSTALL_COMMANDS
 "\\\${RegisterExtension} '$INSTDIR\\\\bin\\\\crossvm.exe' '.crvm' 'CrossLang Executable'\n\
 \\\${RefreshShellIcons}")

# Remove association on uninstall


 set(CPACK_NSIS_EXTRA_INSTALL_COMMANDS
 "\\\${RegisterExtension} '$INSTDIR\\\\bin\\\\crosslang.exe int' '.tcross' 'CrossLang Script'\n\
 \\\${RefreshShellIcons}\n\
 \\\${UnRegisterExtension} '.crvm' 'CrossLang Executable'\n\
 \\\${RefreshShellIcons}")

# Remove association on uninstall
set(CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS
 "\\\${UnRegisterExtension} '.tcross' 'CrossLang Script'\n\
 \\\${RefreshShellIcons}\n\
 \\\${UnRegisterExtension} '.crvm' 'CrossLang Executable'\n\
 \\\${RefreshShellIcons}")

 set(CPACK_NSIS_MUI_ICON "${CMAKE_CURRENT_SOURCE_DIR}/winicon.ico")
set(CPACK_NSIS_MODIFY_PATH ON)
set(CPACK_PACKAGE_VENDOR "Tesses")
set(CPACK_NSIS_CREATE_ICONS_EXTRA "CreateShortCut '$DESKTOP\\\\CrossLang Menu.lnk' '$INSTDIR\\\\bin\\\\crosslang.exe' 'interactive'")
set(CPACK_NSIS_CREATE_ICONS_EXTRA "CreateShortCut '$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\CrossLang Menu.lnk' '$INSTDIR\\\\bin\\\\crosslang.exe' 'interactive'")

include(CPack)