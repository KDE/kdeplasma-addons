project(libplasmaappletdialog)

set(plasmaappletdialog_LIB_SRCS
   plasmaappletdialog.cpp    
)

kde4_add_library(plasmaappletdialog SHARED ${plasmaappletdialog_LIB_SRCS})

target_link_libraries(plasmaappletdialog  ${PLASMA_LIBS} ${KDE4_KDEUI_LIBS})

set_target_properties(plasmaappletdialog PROPERTIES VERSION ${GENERIC_LIB_VERSION}
SOVERSION ${GENERIC_LIB_SOVERSION} )

install(FILES plasmaappletdialog.h plasmaappletdialog_export.h DESTINATION ${INCLUDE_INSTALL_DIR}/)
install(TARGETS plasmaappletdialog  ${INSTALL_TARGETS_DEFAULT_ARGS})
