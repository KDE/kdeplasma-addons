add_subdirectory(fifteenPuzzle)
add_subdirectory(fuzzy-clock)
add_subdirectory(calculator)
add_subdirectory(colorpicker)
add_subdirectory(notes)
add_subdirectory(timer)

find_package(KDEExperimentalPurpose QUIET)
set_package_properties(KDEExperimentalPurpose PROPERTIES DESCRIPTION "EXPERIMENTAL. Support for QuickShare sharing"
                       URL "https://projects.kde.org/projects/playground/libs/purpose"
                       TYPE RUNTIME
                      )

if (KDEExperimentalPurpose_FOUND)
    add_subdirectory(quickshare)
endif()

if(NOT WIN32)
   add_subdirectory(systemloadviewer)
endif(NOT WIN32)

add_subdirectory(kimpanel)
add_subdirectory(showdesktop)

plasma_install_package(webbrowser org.kde.plasma.webbrowser)

add_subdirectory(comic)
add_subdirectory(konsoleprofiles)
add_subdirectory(kickerdash)
add_subdirectory(quicklaunch)

#This applet is disabled because although ported it does not work
# add_subdirectory(eyes)

add_subdirectory(userswitcher)