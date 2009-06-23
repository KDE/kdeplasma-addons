find_package(Eigen2 2.0.3)
macro_log_feature(EIGEN2_FOUND "Eigen2" "Eigen2 enables the Mandelbrot wallpaper plugin." "http://eigen.tuxfamily.org" FALSE "2.0.3")

if(EIGEN2_FOUND)


project(plasma-wallpaper-mandelbrot)

include_directories(${EIGEN2_INCLUDE_DIR})

set(mandelbrot_SRCS
    mandelbrot.cpp
    tile.cpp
    renderthread.cpp
    render_with_arch_defaults.cpp
    detectSSE2.cpp
    mix.cpp
)
kde4_add_ui_files(mandelbrot_SRCS config.ui)

# The x86-specific stuff below does not mean that this code is non-portable!
# The idea is that x86 (32bit) is the ONLY major architecture on which vectorization (SSE2) may or may not be available.
# Other platforms have either ALWAYS vectorization (x86-64, PPC...) or NEVER in which case Eigen takes care of the details.
# Only on x86 do we have to take special care to compile 2 paths, one with SSE and one without, and choose the right one
# at runtime.

# MANDELBROT_ON_X86 will be non-empty if the CPU name contains "86" which we only use to make sure we may ask the
# compiler to enable SSE. At this stage we don't tell the difference between x86 and X86-64, and for 64bit CPUs,
# we don't know if we're in 32bit or 64bit mode. That will be done in the source code using preprocessor symbols.
string(REGEX MATCH "86" MANDELBROT_ON_X86 "${CMAKE_SYSTEM_PROCESSOR}")
if(MANDELBROT_ON_X86)
  set(mandelbrot_SRCS
      ${mandelbrot_SRCS}
      render_with_SSE2_explicitly_enabled.cpp
  )
  if(CMAKE_COMPILER_IS_GNUCXX)
    set(SSE2_CXX_FLAG "-msse2")
  elseif(MSVC)
    set(SSE2_CXX_FLAG "/arch:SSE2")
  else(CMAKE_COMPILER_IS_GNUCXX)
    set(SSE2_CXX_FLAG "")
  endif(CMAKE_COMPILER_IS_GNUCXX)
endif(MANDELBROT_ON_X86)

if(CMAKE_COMPILER_IS_GNUCXX)
  set(NODEBUGINFO_CXX_FLAG "-g0")
else(CMAKE_COMPILER_IS_GNUCXX)
  set(NODEBUGINFO_CXX_FLAG "")
endif(CMAKE_COMPILER_IS_GNUCXX)

# for code using Eigen, function inlining is important. Normally one doesn't need to pass -finline explicitly
# as -O2 is enough, but here there may already be a -fno-inline option passed beforehand, so we use -finline
# to cancel it. Another, separate issue, is that -fno-inline seems breaks the rendering, giving a blank result.
# I'm not sure why, my best guess is an incompatibility between -fno-inline and SSE intrinsics.

if(CMAKE_COMPILER_IS_GNUCXX)
  set(INLINING_CXX_FLAG "-finline")
else(CMAKE_COMPILER_IS_GNUCXX)
  set(INLINING_CXX_FLAG "")
endif(CMAKE_COMPILER_IS_GNUCXX)

if(MANDELBROT_ON_X86)
  set_source_files_properties(
    render_with_SSE2_explicitly_enabled.cpp
    PROPERTIES COMPILE_FLAGS "-O2 -DEIGEN_NO_DEBUG ${INLINING_CXX_FLAG} ${NODEBUGINFO_CXX_FLAG} ${SSE2_CXX_FLAG}"
  )
endif(MANDELBROT_ON_X86)

set_source_files_properties(
  render_with_arch_defaults.cpp
  PROPERTIES COMPILE_FLAGS "-O2 -DEIGEN_NO_DEBUG ${INLINING_CXX_FLAG} ${NODEBUGINFO_CXX_FLAG}"
)

set_source_files_properties(
  mix.cpp
  PROPERTIES COMPILE_FLAGS "-O2 -DEIGEN_NO_DEBUG ${INLINING_CXX_FLAG} ${NODEBUGINFO_CXX_FLAG}"
)

set_source_files_properties(
  renderthread.cpp
  PROPERTIES COMPILE_FLAGS "-O2 -DEIGEN_NO_DEBUG ${INLINING_CXX_FLAG} "
)

kde4_add_plugin(plasma_wallpaper_mandelbrot ${mandelbrot_SRCS})
target_link_libraries(plasma_wallpaper_mandelbrot ${KDE4_PLASMA_LIBS} ${KDE4_KIO_LIBS} ${KDE4_SOLID_LIBS})

install(TARGETS plasma_wallpaper_mandelbrot DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-wallpaper-mandelbrot.desktop DESTINATION ${SERVICES_INSTALL_DIR})

endif(EIGEN2_FOUND)
