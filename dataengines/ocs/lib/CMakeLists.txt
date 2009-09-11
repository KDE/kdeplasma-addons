set(ocsclient_SRCS
   listjob_inst.cpp
   listjobbase.cpp
   providerinitjob.cpp
   eventjob.cpp
   eventparser.cpp
   event.cpp
   activity.cpp
   activityparser.cpp
   person.cpp
   personparser.cpp
   personjob.cpp
   provider.cpp
   postjob.cpp
   folder.cpp
   folderparser.cpp
   message.cpp
   messageparser.cpp
   category.cpp
   categoryparser.cpp
   content.cpp
   contentjob.cpp
   contentparser.cpp
   knowledgebase.cpp
   knowledgebasejob.cpp
   knowledgebaselistjob.cpp
   knowledgebaseparser.cpp
 )



kde4_add_library(ocsclient SHARED ${ocsclient_SRCS})
set_target_properties(ocsclient PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION})

target_link_libraries(ocsclient ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS} ${KDE4_KABC_LIBS})

install(TARGETS ocsclient ${INSTALL_TARGETS_DEFAULT_ARGS})
