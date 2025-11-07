# SPDX-FileCopyrightText: 2025 Kristen McWilliam <kristen@kde.org>
#
# SPDX-License-Identifier: BSD-3-Clause

find_package(ECM ${KF6_MIN_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})
include(KDEInstallDirs)

# Install the metadata file
install(FILES metadata.json DESTINATION ${KDE_INSTALL_DATADIR}/kwin/scripts/virtualdesktopsonlyonprimary/)

# Install the script files
install(DIRECTORY contents DESTINATION ${KDE_INSTALL_DATADIR}/kwin/scripts/virtualdesktopsonlyonprimary)
