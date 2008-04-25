project(liblancelot)

find_package(KDE4 REQUIRED)
include (KDE4Defaults)

find_package(Plasma REQUIRED)
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
)

add_definitions(-DKDE_DEFAULT_DEBUG_AREA=1209)

########### libLancelot ###############

set(lancelot_LIB_SRCS
        Global.cpp

        # Widgets
        widgets/Widget.cpp
        widgets/BasicWidget.cpp
        widgets/ExtenderButton.cpp
        widgets/ToggleExtenderButton.cpp
        widgets/Panel.cpp
        widgets/ResizeBordersPanel.cpp

        # widgets/ScrollPane.cpp

        # widgets/ActionListView.cpp
        # widgets/PassagewayView.cpp
        # widgets/WidgetPositioner.cpp

        # # widgets/CustomListView.cpp

        # Layouts
        layouts/FullBorderLayout.cpp
        # layouts/CardLayout.cpp
        # layouts/ColumnLayout.cpp

        # Models
        # models/ActionListViewModels.cpp
        # models/PassagewayViewModels.cpp
)

kde4_add_library(lancelot SHARED ${lancelot_LIB_SRCS})

target_link_libraries(
   lancelot
   ${KDE4_KIO_LIBS}
   ${KDE4_KFILE_LIBS}
   ${PLASMA_LIBS}
)

set_target_properties(lancelot PROPERTIES VERSION 0.1.0 SOVERSION 1)
install(TARGETS lancelot DESTINATION ${LIB_INSTALL_DIR})

########### install files ###############

set(lancelot_LIB_INCLUDES
   lancelot.h
   lancelot_export.h
   Global.h
)

install(FILES
   ${lancelot_LIB_INCLUDES}
   DESTINATION ${INCLUDE_INSTALL_DIR}/lancelot
)

install(FILES
   widgets/ActionListView.h
   widgets/BasicWidget.h
   #widgets/CustomListView.h
   widgets/ExtenderButton.h
   widgets/Panel.h
   widgets/PassagewayView.h
   widgets/ResizeBordersPanel.h
   widgets/ToggleExtenderButton.h
   widgets/Widget.h
   widgets/WidgetPositioner.h
   DESTINATION ${INCLUDE_INSTALL_DIR}/lancelot/widgets)

install(FILES
   layouts/CardLayout.h
   layouts/ColumnLayout.h
   DESTINATION ${INCLUDE_INSTALL_DIR}/lancelot/layouts)

install(FILES
   models/ActionListViewModels.h
   models/PassagewayViewModels.h
   DESTINATION ${INCLUDE_INSTALL_DIR}/lancelot/models)

# tests
add_subdirectory(tests)
