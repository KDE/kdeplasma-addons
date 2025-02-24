# SPDX-License-Identifier: BSD-3-Clause
# SPDX-FileCopyrightText: 2025 Kai Uwe Broulik <kde@broulik.de>

add_definitions(-DTRANSLATION_DOMAIN="plasma_runner_colors")

kcoreaddons_add_plugin(krunner_colors SOURCES
    colorsrunner.cpp
    colorsrunner.h
    colorcircleiconengine.cpp
    colorcircleiconengine.h
    INSTALL_NAMESPACE "kf6/krunner"
)

target_link_libraries(krunner_colors
    Qt::Gui
    KF6::Runner
    KF6::I18n
)

if(BUILD_TESTING)
    add_subdirectory(autotests)
endif()
