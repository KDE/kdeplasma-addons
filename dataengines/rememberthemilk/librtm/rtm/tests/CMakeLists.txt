
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../)

kde4_add_executable(rtmlogin login.cpp)
target_link_libraries(rtmlogin ${KDE4_KDEUI_LIBS} rtm)

kde4_add_executable(rtmrefresh refresh.cpp)
target_link_libraries(rtmrefresh ${KDE4_KDEUI_LIBS} rtm)
