# Adds KDE definitions so that Lancelot could be built independently
# from the rest of the kdeplasmoids module

find_package(KDE4 REQUIRED)
include (KDE4Defaults)
add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS})



# Finding Lancelot package needed for future - when the libLancelot
# becomes a separate project. It is only temporarily here
find_package(Lancelot REQUIRED)

add_subdirectory(themes)
add_subdirectory(src)
add_subdirectory(icons)
