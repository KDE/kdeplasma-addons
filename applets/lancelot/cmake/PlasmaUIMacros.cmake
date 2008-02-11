set (KDE_PLASMA_UIC_EXECUTABLE ${CMAKE_SOURCE_DIR}/applets/lancelot/tools/puic/puic.py)

message("Plasma UI Compiler is located at: " ${KDE_PLASMA_UIC_EXECUTABLE})

#usage: KDE4_ADD_PLASMA_UI_FILES(foo_SRCS ${ui_files})
macro (KDE4_ADD_PLASMA_UI_FILES _sources )
   foreach (_current_FILE ${ARGN})

      get_filename_component(_tmp_FILE ${_current_FILE} ABSOLUTE)
      get_filename_component(_basename ${_tmp_FILE} NAME_WE)
      set(_header ${CMAKE_CURRENT_BINARY_DIR}/ui_${_basename}.h)
      
      message(${KDE_PLASMA_UIC_EXECUTABLE} "\n  " ${_tmp_FILE} "\n  " ${_header})

      add_custom_command(OUTPUT ${_header}
         COMMAND ${KDE_PLASMA_UIC_EXECUTABLE}
         ${_tmp_FILE}
         ${_header}
      )
      
      list(APPEND ${_sources} ${_header})
      
   endforeach (_current_FILE)
endmacro (KDE4_ADD_PLASMA_UI_FILES)

