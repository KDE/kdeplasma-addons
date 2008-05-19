find_package(KdepimLibs)

if(KDEPIMLIBS_FOUND)
  add_subdirectory(contacts)
endif(KDEPIMLIBS_FOUND)

add_subdirectory(converter)
