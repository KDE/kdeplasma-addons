find_package(PythonInterp REQUIRED)
if (NOT PYTHONINTERP_FOUND)
   message(FATAL_ERROR
      "Python interpreter is not found on this machine. Python is needed to "
      "compile Lancelot. Mind that it is required ONLY for compilation. It is "
      "not needed for running Lancelot" )
endif (NOT PYTHONINTERP_FOUND)

if (NOT KDE_PLASMA_UIC_EXECUTABLE)
   # We don't have Puck path
   set (KDE_PLASMA_UIC_EXECUTABLE ${CMAKE_SOURCE_DIR}/applets/lancelot/tools/puck/puck.py)
endif (NOT KDE_PLASMA_UIC_EXECUTABLE)

message("Plasma UI Compiler (Puck) is located at: " ${KDE_PLASMA_UIC_EXECUTABLE})

# TODO: Change the name of the KDE4_ADD_PLASMA_UI_FILES macro \
# so that it complies with CMake style guides before (if) it  \
# becomes a standard part of Plasma

#usage: PLASMAUI_ADD_UI_FILES(foo_SRCS ${ui_files})
macro (PLASMAUI_ADD_UI_FILES _sources )
   foreach (_current_FILE ${ARGN})

      get_filename_component(_tmp_FILE ${_current_FILE} ABSOLUTE)
      get_filename_component(_basename ${_tmp_FILE} NAME_WE)
      set(_header ${CMAKE_CURRENT_BINARY_DIR}/ui_${_basename}.h)

      message(${KDE_PLASMA_UIC_EXECUTABLE} "\n  " ${_tmp_FILE} "\n  " ${_header})

      add_custom_command(OUTPUT ${_header}
         COMMAND ${KDE_PLASMA_UIC_EXECUTABLE}
         ${_tmp_FILE}
         ${_header}

         DEPENDS ${_current_FILE}
      )

      list(APPEND ${_sources} ${_header})

   endforeach (_current_FILE)
endmacro (PLASMAUI_ADD_UI_FILES)

