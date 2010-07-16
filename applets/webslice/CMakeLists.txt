project(webslice)

include_directories( ${QT_INCLUDES} )



set(webslice_SRCS
    main.cpp
    kgraphicswebslice.cpp
    kwebslice.cpp
)

KDE4_ADD_EXECUTABLE(webslice ${webslice_SRCS})

target_link_libraries(webslice
    ${QT_QTCORE_LIBRARY}
    ${QT_QTNETWORK_LIBRARY}
    ${QT_QTGUI_LIBRARY}
    ${QT_QTWEBKIT_LIBRARY}
    ${QT_QTSCRIPT_LIBRARY}
)

add_subdirectory(plasma)
