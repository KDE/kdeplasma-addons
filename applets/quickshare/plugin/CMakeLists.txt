install(FILES qmldir DESTINATION ${QML_INSTALL_DIR}/org/kde/plasma/private/purpose)
add_library(purposeplugin SHARED purposeplugin.cpp)

target_link_libraries(purposeplugin
                        Qt5::Quick
                        KF5::CoreAddons)

install(TARGETS purposeplugin DESTINATION ${QML_INSTALL_DIR}/org/kde/plasma/private/purpose)
