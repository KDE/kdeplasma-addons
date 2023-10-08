# SPDX-FileCopyrightText: 2022 Vlad Zahorodnii <vlad.zahorodnii@kde.org>
#
# SPDX-License-Identifier: BSD-3-Clause

add_subdirectory(kcm)

kcoreaddons_add_plugin(kwin4_effect_cube INSTALL_NAMESPACE "kwin/effects/plugins/")
target_sources(kwin4_effect_cube PRIVATE
    cubeeffect.cpp
    main.cpp
)
target_compile_definitions(kwin4_effect_cube PRIVATE
    -DTRANSLATION_DOMAIN="kwin_effect_cube"
)
kconfig_add_kcfg_files(kwin4_effect_cube cubeconfig.kcfgc)

target_link_libraries(kwin4_effect_cube
    Qt::Core
    Qt::Gui
    Qt::Quick

    KF6::ConfigCore
    KF6::ConfigGui
    KF6::CoreAddons
    KF6::GlobalAccel
    KF6::I18n
    KF6::WindowSystem

    KWinEffects::kwineffects
)

install(DIRECTORY qml DESTINATION ${KDE_INSTALL_DATADIR}/kwin/effects/cube)
