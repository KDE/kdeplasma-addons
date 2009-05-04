set(ocsclient_SRCS
   activity.cpp
   activityparser.cpp
   activitylistjob.cpp
   person.cpp
   personparser.cpp
   personjob.cpp
   personlistjob.cpp
   ocsapi.cpp
   postjob.cpp
   folder.cpp
   folderlistjob.cpp
   folderparser.cpp
   message.cpp
   messagelistjob.cpp
   messageparser.cpp
   category.cpp
   categorylistjob.cpp
   categoryparser.cpp
   content.cpp
   contentjob.cpp
   contentlistjob.cpp
   contentparser.cpp
   knowledgebase.cpp
   knowledgebasejob.cpp
   knowledgebaselistjob.cpp
   knowledgebaseparser.cpp
 )


find_package(KDE4 REQUIRED)
include (KDE4Defaults)

kde4_add_library(ocsclient SHARED ${ocsclient_SRCS})
set_target_properties(ocsclient PROPERTIES VERSION ${KDE_GENERIC_LIB_VERSION} SOVERSION ${KDE_GENERIC_LIB_SOVERSION})

target_link_libraries(ocsclient ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS} ${KDE4_KABC_LIBS})

install(TARGETS ocsclient DESTINATION ${LIB_INSTALL_DIR})
