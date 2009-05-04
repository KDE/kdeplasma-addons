project(plasma-engine-rtm) 

#find_package(RTM REQUIRED) # Will be needed again later
set(RTM_LIBRARY rtm)
 
add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS})

include_directories(
   ${CMAKE_CURRENT_SOURCE_DIR}
   ${CMAKE_CURRENT_BINARY_DIR}
   ${KDE4_INCLUDES}
   #${RTM_INCLUDE_DIR}
   )
 
set(rtm_engine_SRCS 
    rtmengine.cpp
    
    taskssource.cpp
    listssource.cpp
    tasksource.cpp
    listsource.cpp
    
    authservice.cpp
    tasksservice.cpp
    taskservice.cpp
  )
 
kde4_add_plugin(plasma_engine_rtm ${rtm_engine_SRCS})
target_link_libraries(plasma_engine_rtm
                      ${KDE4_KDECORE_LIBS}
                      ${KDE4_PLASMA_LIBS}
		      ${QT_QTNETWORK_LIBRARY}
		      ${RTM_LIBRARY})
 
install(TARGETS plasma_engine_rtm
        DESTINATION ${PLUGIN_INSTALL_DIR})
 
install(FILES plasma-engine-rtm.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})

install(FILES rtmauth.operations rtmtasks.operations rtmtask.operations
	DESTINATION ${DATA_INSTALL_DIR}/plasma/services)
