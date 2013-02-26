add_subdirectory(comic)

if (QCA2_FOUND AND QJSON_FOUND AND QTOAUTH_FOUND)
      add_subdirectory(microblog)
endif(QCA2_FOUND AND QJSON_FOUND AND QTOAUTH_FOUND)

if(LIBATTICA_FOUND)
  add_subdirectory(ocs)
endif(LIBATTICA_FOUND)

add_subdirectory(potd)
add_subdirectory(rememberthemilk)
add_subdirectory(kdeobservatory)
add_subdirectory(kimpanel)
add_subdirectory(konsoleprofiles)
add_subdirectory(konqprofiles)
