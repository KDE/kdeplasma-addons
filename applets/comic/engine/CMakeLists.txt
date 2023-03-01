# SPDX-License-Identifier: BSD-3-Clause
# SPDX-FileCopyrightText: 2022 Alexander Lohnau <alexander.lohnau@gmx.de>

add_library(plasma_engine_comic STATIC)

set_property(TARGET plasma_engine_comic PROPERTY POSITION_INDEPENDENT_CODE ON)

ecm_qt_declare_logging_category(plasma_engine_comic
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

target_sources(plasma_engine_comic PRIVATE
    comicprovider.cpp
    cachedprovider.cpp
    comic.cpp
    comicproviderkross.cpp
    comicproviderwrapper.cpp
)
target_link_libraries(plasma_engine_comic
    Qt::Qml
    KF6::WidgetsAddons
    KF6::Package
    KF6::I18n
    KF6::KIOCore
)
if (QT_MAJOR_VERSION EQUAL "6")
    target_link_libraries(plasma_engine_comic Qt::Core5Compat) # for QTextCodec
endif()

kcoreaddons_add_plugin(plasma_packagestructure_comic SOURCES comic_package.cpp INSTALL_NAMESPACE "kpackage/packagestructure")
target_link_libraries(plasma_packagestructure_comic
    KF6::Package
    KF6::I18n
)
set_target_properties(plasma_packagestructure_comic PROPERTIES OUTPUT_NAME plasma_comic)
install(FILES plasma-comic.desktop DESTINATION ${KDE_INSTALL_KSERVICETYPESDIR})
