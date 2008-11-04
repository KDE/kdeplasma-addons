project(plasma-kolourpicker)

set(KOLOURPICKER_X11_LIB FALSE)
if(X11_FOUND AND X11_Xutil_FOUND)
   set(KOLOURPICKER_X11_LIB TRUE)
   include_directories(${X11_Xutil_INCLUDE_PATH})
endif(X11_FOUND AND X11_Xutil_FOUND)

configure_file(
   ${CMAKE_CURRENT_SOURCE_DIR}/config-kolourpicker.h.cmake
   ${CMAKE_CURRENT_BINARY_DIR}/config-kolourpicker.h
)

set(kolourpicker_SRCS
    kolourpicker.cpp
)

kde4_add_plugin(plasma_applet_kolourpicker ${kolourpicker_SRCS})
target_link_libraries(plasma_applet_kolourpicker ${KDE4_PLASMA_LIBS} ${KDE4_KIO_LIBS})
if(KOLOURPICKER_X11_LIB)
   target_link_libraries(plasma_applet_kolourpicker ${X11_X11_LIB})
endif(KOLOURPICKER_X11_LIB)

install(TARGETS plasma_applet_kolourpicker DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-kolourpicker-default.desktop DESTINATION ${SERVICES_INSTALL_DIR})
