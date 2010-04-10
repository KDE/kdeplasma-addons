add_subdirectory(audioplayercontrol)
add_subdirectory(browserhistory)
add_subdirectory(converter)
add_subdirectory(katesessions)
add_subdirectory(konquerorsessions)
add_subdirectory(kopete)
add_subdirectory(mediawiki)
add_subdirectory(spellchecker)
add_subdirectory(characters)

if(KDEPIMLIBS_FOUND)
add_subdirectory(contacts)
endif(KDEPIMLIBS_FOUND)

if(NOT WIN32)
 add_subdirectory(konsolesessions)
endif(NOT WIN32)

