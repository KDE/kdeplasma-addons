project(librtm)

include_directories(${KDE4_INCLUDES} ${QT4_INCLUDES} ${QT_INCLUDES} ${QT_INCLUDE_DIR} ${QT_QTNETWORK_INCLUDE_DIR} ${QT_QTXML_INCLUDE_DIR})

set( rtm_LIB_HEADERS
        rtm_export.h
        rtm.h
        request.h
        auth.h
        task.h
        session.h
        list.h
        note.h
        xmlreaders.h
        )

set( rtm_SRCS 
        session.cpp
        task.cpp
        list.cpp
        request.cpp
        auth.cpp
        xmlreaders.cpp
    )

kde4_add_library(rtm SHARED ${rtm_SRCS})

target_link_libraries(rtm ${QT_QTNETWORK_LIBRARY} ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS} ${KDE4_KHTML_LIBS})


#### FOR 4.3 we are not including the library as a stable platform, hence no exporting of headers

install(TARGETS rtm ${INSTALL_TARGETS_DEFAULT_ARGS})
#install(TARGETS rtm EXPORT RtmExports ${INSTALL_TARGETS_DEFAULT_ARGS})
#install(EXPORT RtmExports DESTINATION ${DATA_INSTALL_DIR}/cmake/modules
#                             FILE RtmExportedLibraryTargets.cmake )

#install( FILES ${rtm_LIB_HEADERS}
#  DESTINATION ${INCLUDE_INSTALL_DIR}/rtm
#  COMPONENT Devel
#)

#install(FILES FindRTM.cmake DESTINATION ${DATA_INSTALL_DIR}/cmake/modules)

add_subdirectory(tests)
