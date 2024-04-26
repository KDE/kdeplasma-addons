# SPDX-License-Identifier: BSD-3-Clause
# SPDX-FileCopyrightText: 2024 Fushan Wen <qydwhotmail@gmail.com>
ecm_add_qml_module(colorpickerplugin URI org.kde.plasma.private.colorpicker GENERATE_PLUGIN_SOURCE)

target_sources(colorpickerplugin PRIVATE
    colorpickerplugin.cpp
)
target_link_libraries(colorpickerplugin PRIVATE Qt::Gui Qt::Qml)
ecm_finalize_qml_module(colorpickerplugin)
