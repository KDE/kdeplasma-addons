
set(groupingcontainment_SRCS
        abstractgroup.cpp
        groupfactory.cpp
        groupingcontainment.cpp
        handle.cpp
        freehandle.cpp
        gridhandle.cpp
        spacer.cpp

        groups/floatinggroup.cpp
        groups/gridgroup.cpp
        groups/flowgroup.cpp
        groups/stackinggroup.cpp
        groups/tabbinggroup.cpp)


kde4_add_ui_files(groupingcontainment_SRCS groups/tabbinggroup_config.ui)
kde4_add_library(plasma_groupingcontainment SHARED ${groupingcontainment_SRCS})
target_link_libraries(plasma_groupingcontainment ${KDE4_PLASMA_LIBS})

set_target_properties(plasma_groupingcontainment PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION})
install(TARGETS plasma_groupingcontainment EXPORT kdeworkspaceLibraryTargets ${INSTALL_TARGETS_DEFAULT_ARGS})
