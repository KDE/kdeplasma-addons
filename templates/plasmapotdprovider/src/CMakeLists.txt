# SPDX-FileCopyrightText: %{CURRENT_YEAR} %{AUTHOR} <%{EMAIL}>
# SPDX-License-Identifier: BSD-2-Clause

add_library(plasma_potd_%{APPNAMELC} MODULE %{APPNAMELC}.cpp)
target_link_libraries(plasma_potd_%{APPNAMELC}
    Plasma::PotdProvider
    KF6::KIOCore
)

install(TARGETS plasma_potd_%{APPNAMELC} DESTINATION ${KDE_INSTALL_PLUGINDIR}/potd)
