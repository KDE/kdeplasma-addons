project(liblancelot)

add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS})

########### define variables ##########

set(LANCELOT_INCLUDE_DIR
        "${CMAKE_CURRENT_SOURCE_DIR}/.."
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
        ${PLASMA_INCLUDE_DIR}
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

        widgets/ScrollBar.cpp
        widgets/ScrollPane.cpp

        widgets/ActionListView.cpp
        widgets/PassagewayView.cpp
        widgets/WidgetPositioner.cpp

        # # widgets/CustomListView.cpp

        # Layouts
        layouts/FullBorderLayout.cpp
        layouts/NodeLayout.cpp
        layouts/CardLayout.cpp
        layouts/ColumnLayout.cpp
        layouts/FlipLayout.cpp

        # Models
        models/ActionListViewModels.cpp
        models/PassagewayViewModels.cpp
)

kde4_add_library(lancelot SHARED ${lancelot_LIB_SRCS})

target_link_libraries(
   lancelot
   ${KDE4_KIO_LIBS}
   ${KDE4_KFILE_LIBS}
   ${PLASMA_LIBS}
)

set_target_properties(lancelot PROPERTIES VERSION 0.1.0 SOVERSION 1)
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
   #widgets/CustomListView.h
   widgets/ExtenderButton.h
   widgets/Panel.h
   widgets/PassagewayView.h
   widgets/ResizeBordersPanel.h
   widgets/Widget.h
   widgets/WidgetPositioner.h
   widgets/HoverIcon.h
   DESTINATION ${INCLUDE_INSTALL_DIR}/lancelot/widgets COMPONENT Devel)

install(FILES
   layouts/CardLayout.h
   layouts/ColumnLayout.h
   DESTINATION ${INCLUDE_INSTALL_DIR}/lancelot/layouts COMPONENT Devel)

install(FILES
   models/ActionListViewModels.h
   models/PassagewayViewModels.h
   DESTINATION ${INCLUDE_INSTALL_DIR}/lancelot/models COMPONENT Devel)

# tests
add_subdirectory(tests)
