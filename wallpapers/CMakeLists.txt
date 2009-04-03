add_subdirectory(pattern)

if(MARBLE_FOUND)
    message(STATUS "Marble Plasma wallpaper will be built.")
    add_subdirectory(marble)
else(MARBLE_FOUND)
    message(STATUS "Marble Plasma wallpaper won't be built.")
endif(MARBLE_FOUND)
