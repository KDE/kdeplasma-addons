project(liblancelot)

find_package(KDE4 REQUIRED)
include (KDE4Defaults)
add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS})

########### define variables ##########

set(LANCELOT_INCLUDE_DIR
        "${CMAKE_CURRENT_SOURCE_DIR}/.."
        "${CMAKE_CURRENT_SOURCE_DIR}/"
        CACHE STRING
        "Location of liblancelot headers"
        FORCE)

set(LANCELOT_LIBS
        "lancelot"
        CACHE STRING
        "Location of liblancelot binary"
        FORCE)

include_directories(
        ${CMAKE_SOURCE_DIR}
        ${CMAKE_BINARY_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_BINARY_DIR}
        ${KDE4_INCLUDES}

        ${LANCELOT_INCLUDE_DIR}
)

add_definitions(-DKDE_DEFAULT_DEBUG_AREA=1209)

########### libLancelot ###############

set(lancelot_LIB_SRCS
        Global.cpp

        # Widgets
        widgets/Widget.cpp
        widgets/BasicWidget.cpp
        widgets/ExtenderButton.cpp
        widgets/Panel.cpp
        widgets/ResizeBordersPanel.cpp
        widgets/HoverIcon.cpp
        widgets/TabBar.cpp

        widgets/ScrollBar.cpp
        widgets/ScrollPane.cpp
        widgets/kineticscroll.cpp

        widgets/ActionListView.cpp
        widgets/PassagewayView.cpp

        widgets/CustomListView.cpp

        widgets/PopupList.cpp
        widgets/PopupMenu.cpp

        widgets/CustomItemBackground_p.cpp

        # Layouts
        layouts/FullBorderLayout.cpp
        layouts/NodeLayout.cpp
        layouts/CardLayout.cpp
        layouts/ColumnLayout.cpp
        layouts/FlipLayout.cpp

        # Models
        models/ActionListModel.cpp
        models/StandardActionListModel.cpp
        models/MergedActionListModel.cpp

        models/ActionTreeModel.cpp
        models/StandardActionTreeModel.cpp
        models/ActionTreeModelProxy.cpp

        models/PlasmaServiceListModel.cpp

        # models/ActionListViewModels.cpp
        # models/PassagewayViewModels.cpp
)

kde4_add_library(lancelot SHARED ${lancelot_LIB_SRCS})

target_link_libraries(
   lancelot
   ${KDE4_KIO_LIBS}
   ${KDE4_KFILE_LIBS}
   ${KDE4_PLASMA_LIBS}
)

set_target_properties(lancelot PROPERTIES VERSION 1.9.0 SOVERSION 2)
install(TARGETS lancelot ${INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############

set(lancelot_LIB_INCLUDES
   lancelot.h
   lancelot_export.h
   Global.h
)

install(FILES
   ${lancelot_LIB_INCLUDES}
   DESTINATION ${INCLUDE_INSTALL_DIR}/lancelot COMPONENT Devel
)

install(FILES
   widgets/ActionListView.h
   widgets/BasicWidget.h
   widgets/CustomListView.h
   widgets/ExtenderButton.h
   widgets/Panel.h
   widgets/PassagewayView.h
   widgets/ResizeBordersPanel.h
   widgets/Widget.h
   widgets/HoverIcon.h
   widgets/TabBar.h
   widgets/PopupList.h
   widgets/PopupMenu.h
   widgets/ScrollPane.h
   widgets/ScrollBar.h
   DESTINATION ${INCLUDE_INSTALL_DIR}/lancelot/widgets COMPONENT Devel)

install(FILES
   layouts/FullBorderLayout.h
   layouts/NodeLayout.h
   layouts/CardLayout.h
   layouts/ColumnLayout.h
   layouts/FlipLayout.h
   DESTINATION ${INCLUDE_INSTALL_DIR}/lancelot/layouts COMPONENT Devel)

install(FILES
   models/ActionListModel.h
   models/StandardActionListModel.h
   models/MergedActionListModel.h

   models/ActionTreeModel.h
   models/StandardActionTreeModel.h
   models/ActionTreeModelProxy.h

   models/PlasmaServiceListModel.h

   # models/ActionListViewModels.h
   # models/PassagewayViewModels.h
   DESTINATION ${INCLUDE_INSTALL_DIR}/lancelot/models COMPONENT Devel)

install(FILES
   Lancelot/ActionListModel
   Lancelot/ActionListView
   Lancelot/ActionTreeModel
   Lancelot/ActionTreeModelProxy
   Lancelot/BasicWidget
   Lancelot/CardLayout
   Lancelot/ColumnLayout
   Lancelot/CustomListView
   Lancelot/ExtenderButton
   Lancelot/FlipLayout
   Lancelot/FullBorderLayout
   Lancelot/Global
   Lancelot/HoverIcon
   Lancelot/Lancelot
   Lancelot/MergedActionListModel
   Lancelot/NodeLayout
   Lancelot/Panel
   Lancelot/PassagewayView
   Lancelot/PlasmaServiceListModel
   Lancelot/PopupList
   Lancelot/PopupMenu
   Lancelot/TabBar
   Lancelot/ResizeBordersPanel
   Lancelot/ScrollBar
   Lancelot/ScrollPane
   Lancelot/StandardActionListModel
   Lancelot/StandardActionTreeModel
   Lancelot/Widget
   DESTINATION ${INCLUDE_INSTALL_DIR}/KDE/Lancelot COMPONENT Devel)

# tests
# add_subdirectory(tests)
