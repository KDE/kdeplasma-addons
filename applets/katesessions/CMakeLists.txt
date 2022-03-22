kcoreaddons_add_plugin(plasma_addons_engine_katesessions INSTALL_NAMESPACE "plasma/dataengine")
target_compile_definitions(plasma_addons_engine_katesessions PRIVATE TRANSLATION_DOMAIN="plasma_applet_org.kde.plasma.katesessions")

target_link_libraries(
  plasma_addons_engine_katesessions 
  PRIVATE
    KF5::I18n
    KF5::Plasma
    KF5::KIOGui
    Qt::Widgets
)

target_sources(
  plasma_addons_engine_katesessions 
  PRIVATE
    katesessionsengine.cpp 
    katesessionsmodel.cpp 
    katesessionsjob.cpp 
    katesessionsservice.cpp
)

plasma_install_package(applet org.kde.plasma.addons.katesessions)
kcoreaddons_desktop_to_json(plasma_addons_engine_katesessions plasma-dataengine-katesessions.desktop)

install(FILES plasma-dataengine-katesessions.desktop DESTINATION ${KDE_INSTALL_KSERVICESDIR})
install(FILES org.kde.plasma.katesessions.operations DESTINATION ${PLASMA_DATA_INSTALL_DIR}/services)

