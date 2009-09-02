add_subdirectory(mandelbrot)
add_subdirectory(pattern)
add_subdirectory(virus)

if(MARBLE_FOUND)
    add_subdirectory(marble)
endif(MARBLE_FOUND)

if(KUNITCONVERSION_FOUND)
  add_subdirectory(weather)
endif(KUNITCONVERSION_FOUND)
