# SPDX-License-Identifier: BSD-3-Clause
# SPDX-FileCopyrightText: 2022 Alexander Lohnau <alexander.lohnau@gmx.de>
set(LOGGING_SRCS)
ecm_qt_declare_logging_category(LOGGING_SRCS
    HEADER comic_debug.h
    IDENTIFIER PLASMA_COMIC
    CATEGORY_NAME ork.kde.plasma.comic
    DESCRIPTION "Plasma Comic Engine"
    EXPORT PLASMA_COMIC
)
ecm_qt_install_logging_categories(
    EXPORT PLASMA_COMIC
    FILE plasma_comic.categories
    DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR}
)

set(comic_engine_SRCS
    cachedprovider.cpp
    comic.cpp
    comicproviderkross.cpp
    comicproviderwrapper.cpp
    ${LOGGING_SRCS}
)

add_library(plasma_engine_comic STATIC ${comic_engine_SRCS})

target_link_libraries(plasma_engine_comic plasmacomicprovidercore
    Qt::Qml
    KF5::WidgetsAddons
    KF5::Package
    KF5::I18n
)

kcoreaddons_add_plugin(plasma_packagestructure_comic SOURCES comic_package.cpp INSTALL_NAMESPACE "kpackage/packagestructure")
target_link_libraries(plasma_packagestructure_comic
    KF5::Package
    KF5::I18n
)
set_target_properties(plasma_packagestructure_comic PROPERTIES OUTPUT_NAME plasma_comic)

########### plugin core library ############

set(comic_provider_core_SRCS
  comicprovider.cpp
  ${LOGGING_SRCS}
)

add_library(plasmacomicprovidercore STATIC ${comic_provider_core_SRCS})
target_link_libraries(plasmacomicprovidercore
    Qt::Gui
    KF5::KIOCore
    KF5::I18n
)
