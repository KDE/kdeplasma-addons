project(CharacterRunner)

set(KDE_MIN_VERSION "4.3.60") # for the < 4.2 macro
find_package(KDE4 4.3.60 REQUIRED)
include (KDE4Defaults)
include (MacroLibrary)

add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${KDE4_INCLUDES})

# We add our source code here
set(krunner_charrunner_SRCS charrunner.cpp)
set(kcm_krunner_charrunner_SRCS charrunner_config.cpp)

kde4_add_ui_files(kcm_krunner_charrunner_SRCS charrunner_config.ui)
kde4_add_plugin(kcm_krunner_charrunner ${kcm_krunner_charrunner_SRCS})
target_link_libraries(kcm_krunner_charrunner
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KUTILS_LIBS} 
    ${QT_QTCORE_LIBRARY}
    ${QT_QTGUI_LIBRARY}
    )

# Now make sure all files get to the right place
kde4_add_plugin(krunner_charrunner ${krunner_charrunner_SRCS})
target_link_libraries(krunner_charrunner ${KDE4_PLASMA_LIBS} ${KDE4_KIO_LIBS})

# Install the library and .desktop file
install(TARGETS krunner_charrunner kcm_krunner_charrunner DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES CharacterRunner.desktop CharRunner_config.desktop DESTINATION ${SERVICES_INSTALL_DIR}) 
