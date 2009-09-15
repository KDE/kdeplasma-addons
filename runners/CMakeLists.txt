add_subdirectory(browserhistory)
add_subdirectory(converter)
add_subdirectory(katesessions)
add_subdirectory(konquerorsessions)
add_subdirectory(spellchecker)

if(KDEPIMLIBS_FOUND})
add_subdirectory(contacts)
endif(KDEPIMLIBS_FOUND})

if(NOT WIN32)
 add_subdirectory(konsolesessions)
endif(NOT WIN32)

