add_subdirectory(fifteenPuzzle)
add_subdirectory(fuzzy-clock)
add_subdirectory(calculator)
add_subdirectory(notes)
add_subdirectory(timer)

if(NOT WIN32)
   add_subdirectory(systemloadviewer)
endif(NOT WIN32)

add_subdirectory(kimpanel)

# This plasmoid is disabled because although ported it does not actually work on Qt5.3
# Re-enable in Plasma 5.2 AFTER testing drag & drop
# plasma_install_package(quickshare org.kde.plasma.quickshare)

#This applet is disabled because although ported it does not work
# add_subdirectory(eyes)



