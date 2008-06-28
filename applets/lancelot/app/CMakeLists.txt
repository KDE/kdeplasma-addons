# Adds KDE definitions so that Lancelot could be built independently
# from the rest of the kdeplasmoids module

find_package(KDE4 REQUIRED)
include (KDE4Defaults)
add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS})

add_subdirectory(themes)
add_subdirectory(src)
add_subdirectory(icons)
