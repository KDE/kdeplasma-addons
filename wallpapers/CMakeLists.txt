add_subdirectory(mandelbrot)
add_subdirectory(pattern)
add_subdirectory(potd)
add_subdirectory(virus)
add_subdirectory(weather)
add_subdirectory(qmlwallpapers)

if(MARBLE_FOUND)
    add_subdirectory(marble)
endif(MARBLE_FOUND)

