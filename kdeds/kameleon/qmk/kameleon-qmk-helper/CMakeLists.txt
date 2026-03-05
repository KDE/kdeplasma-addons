# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: 2026 Harald Sitter <sitter@kde.org>

include(FetchContent)

FetchContent_Declare(
    Corrosion
    GIT_REPOSITORY https://github.com/corrosion-rs/corrosion.git
)
FetchContent_MakeAvailable(Corrosion)

# Import targets defined in a package or workspace manifest `Cargo.toml` file
corrosion_import_crate(MANIFEST_PATH Cargo.toml)

install(PROGRAMS ${CMAKE_BINARY_DIR}/bin/kameleon-qmk-helper DESTINATION ${KDE_INSTALL_LIBEXECDIR_KF})

install(FILES org.kde.kameleon.qmk.helper.conf DESTINATION ${KDE_INSTALL_DBUSDIR}/system.d)
install(FILES org.kde.kameleon.qmk.helper.policy DESTINATION ${KDE_INSTALL_DATADIR}/polkit-1/actions)
configure_file(org.kde.kameleon.qmk.helper.service.in ${CMAKE_CURRENT_BINARY_DIR}/org.kde.kameleon.qmk.helper.service)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/org.kde.kameleon.qmk.helper.service DESTINATION ${KDE_INSTALL_DBUSSYSTEMSERVICEDIR})
