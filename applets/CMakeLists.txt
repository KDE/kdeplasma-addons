plasma_install_package(activitypager org.kde.plasma.activitypager)
add_subdirectory(fifteenPuzzle)
add_subdirectory(fuzzy-clock)
add_subdirectory(binary-clock)
add_subdirectory(calculator)
add_subdirectory(colorpicker)
add_subdirectory(mediaframe)
add_subdirectory(notes)
add_subdirectory(timer)
add_subdirectory(dict)

add_subdirectory(quickshare)

if(NOT WIN32)
    # uses the command line tool 'quota', does not make sense on Windows
    add_subdirectory(diskquota)

    add_subdirectory(systemloadviewer)
endif(NOT WIN32)

add_subdirectory(showdesktop)
add_subdirectory(minimizeall)

plasma_install_package(webbrowser org.kde.plasma.webbrowser)

add_subdirectory(comic)
plasma_install_package(konsoleprofiles/package org.kde.plasma.konsoleprofiles)
plasma_install_package(kickerdash org.kde.plasma.kickerdash)
add_subdirectory(quicklaunch)

#This applet is disabled because although ported it does not work
# add_subdirectory(eyes)

add_subdirectory(userswitcher)
add_subdirectory(weather)
add_subdirectory(grouping)
