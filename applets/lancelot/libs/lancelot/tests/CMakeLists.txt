project(LancelotTests)

add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS})

# include(PlasmaUIMacros)

include_directories(
   ${CMAKE_SOURCE_DIR}
   ${CMAKE_BINARY_DIR}
   ${CMAKE_CURRENT_SOURCE_DIR}
   ${CMAKE_CURRENT_BINARY_DIR}
   ${KDE4_INCLUDES}
   
   ${LANCELOT_INCLUDE_DIR}
)

# Lancelot application files
set(plasma_lancelot_tests_SRCS
  LancelotTestWindow.cpp
  LancelotTestApplication.cpp
  main.cpp
)

kde4_add_executable(lancelot-test ${plasma_lancelot_tests_SRCS})

target_link_libraries(lancelot-test ${KDE4_KDEUI_LIBS} ${KDE4_PLASMA_LIBS} ${LANCELOT_LIBS})

install(TARGETS lancelot-test ${INSTALL_TARGETS_DEFAULT_ARGS} )
