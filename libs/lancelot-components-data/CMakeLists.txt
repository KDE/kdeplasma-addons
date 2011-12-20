project(lancelotcomponentsdata)

include(KDE4Defaults)

set(lancelotcomponentsdata_SRCS
    lancelotcomponentsdataplugin.cpp
    BaseModel.cpp
    FavoriteApplications.cpp
    FolderModel.cpp
    DirModel.cpp
    )

INCLUDE_DIRECTORIES(
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_BINARY_DIR}
    ${KDE4_INCLUDES}
    )

qt4_automoc(${lancelotcomponentsdata_SRCS})


add_library(
    lancelotcomponentsdataplugin
    SHARED
    ${lancelotcomponentsdata_SRCS}
    )

target_link_libraries(
    lancelotcomponentsdataplugin
    ${QT_QTCORE_LIBRARY}
    ${QT_QTGUI_LIBRARY}
    ${QT_QTDECLARATIVE_LIBRARY}
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KFILE_LIBS}
    ${KDE4_KIO_LIBS}
    ${KDE4_PLASMA_LIBS}
    ${KDE4_SOLID_LIBS}
    )

install(
    TARGETS
    lancelotcomponentsdataplugin
    DESTINATION
    ${IMPORTS_INSTALL_DIR}/org/kde/lancelot/components/data
    )

install(
    FILES
    qmldir
    DESTINATION
    ${IMPORTS_INSTALL_DIR}/org/kde/lancelot/components/data
    )
