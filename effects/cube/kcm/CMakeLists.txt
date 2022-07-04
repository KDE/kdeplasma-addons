# SPDX-FileCopyrightText: 2022 Vlad Zahorodnii <vlad.zahorodnii@kde.org>
#
# SPDX-License-Identifier: BSD-3-Clause

set(kwin_cube_config_SOURCES cubeeffectkcm.cpp)
ki18n_wrap_ui(kwin_cube_config_SOURCES cubeeffectkcm.ui)
qt5_add_dbus_interface(kwin_cube_config_SOURCES ${KWIN_EFFECTS_INTERFACE} kwineffects_interface)

kcoreaddons_add_plugin(kwin_cube_config INSTALL_NAMESPACE "kwin/effects/configs" SOURCES ${kwin_cube_config_SOURCES})
target_compile_definitions(kwin_cube_config PRIVATE
    -DTRANSLATION_DOMAIN="kwin_effect_cube"
)
kconfig_add_kcfg_files(kwin_cube_config ../cubeconfig.kcfgc)
target_link_libraries(kwin_cube_config
    KF5::ConfigCore
    KF5::ConfigGui
    KF5::ConfigWidgets
    KF5::CoreAddons
    KF5::GlobalAccel
    KF5::I18n
    KF5::XmlGui
)
