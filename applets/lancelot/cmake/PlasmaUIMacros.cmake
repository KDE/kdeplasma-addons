
#usage: KDE4_ADD_PLASMA_UI_FILES(foo_SRCS ${ui_files})
macro (KDE4_ADD_PLASMA_UI_FILES _sources )
   message("asdpo ############################################################1")
   foreach (_current_FILE ${ARGN})
      message("asdpo ############################################################3")

      get_filename_component(_tmp_FILE ${_current_FILE} ABSOLUTE)
      message(${_tmp_FILE})
      
      get_filename_component(_basename ${_tmp_FILE} NAME_WE)
      message(${_basename})
      
      set(_header ${CMAKE_CURRENT_BINARY_DIR}/ui_${_basename}.h)
      message(${_header})

      message("asdpo ############################################################3")
   endforeach (_current_FILE)
endmacro (KDE4_ADD_PLASMA_UI_FILES)

