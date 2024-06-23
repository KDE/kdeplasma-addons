find_package(KWin REQUIRED)
find_package(KF6 REQUIRED COMPONENTS Config)

kcoreaddons_add_plugin(reflectcursor INSTALL_NAMESPACE "kwin/effects/plugins")

target_compile_definitions(reflectcursor PRIVATE -DTRANSLATION_DOMAIN=\"effect_reflectcusor\")

target_sources(reflectcursor PRIVATE
    main.cpp
    reflectcursor.cpp
)

kconfig_add_kcfg_files(reflectcursor
    reflectcursorconfig.kcfgc
)

target_link_libraries(reflectcursor PRIVATE
    KF6::ConfigGui

    KWin::kwin
)
