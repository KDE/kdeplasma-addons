add_subdirectory(converter)
add_subdirectory(datetime)
add_subdirectory(katesessions)
# Disabled for now due to Milou not properly handling the bigger timeout, see kde bug #389611
# add_subdirectory(mediawiki)
add_subdirectory(spellchecker)
add_subdirectory(characters)
add_subdirectory(dictionary)

if(NOT WIN32)
    add_subdirectory(konsoleprofiles)
endif(NOT WIN32)
