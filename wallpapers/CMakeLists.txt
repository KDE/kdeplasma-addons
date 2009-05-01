add_subdirectory(mandelbrot)
add_subdirectory(pattern)
add_subdirectory(weather)
add_subdirectory(virus)

if(MARBLE_FOUND)
    add_subdirectory(marble)
endif(MARBLE_FOUND)

