project(plasma-frame)
include (MacroLogFeature)

macro_optional_find_package(Kexiv2)
macro_log_feature(KEXIV2_FOUND "libkexiv2" "Library to access EXIF information" "http://www.kde.org" FALSE "0.2.0" "RECOMENDED: Enables automatic rotation for frame applet" )

macro_display_feature_log()

set(frame_SRCS
    frame.cpp
    slideshow.cpp
    picture.cpp
    configdialog.cpp)

kde4_add_ui_files(frame_SRCS imageSettings.ui appearanceSettings.ui)

macro_bool_to_01(KEXIV2_FOUND HAVE_KEXIV2)

kde4_add_plugin(plasma_applet_frame ${frame_SRCS})


if(HAVE_KEXIV2)
    include_directories( ${KEXIV2_INCLUDES} )
    set_source_files_properties(picture.cpp PROPERTIES 
                               COMPILE_FLAGS -DHAVE_KEXIV2)
    target_link_libraries(plasma_applet_frame ${KDE4_PLASMA_LIBS} ${KDE4_KIO_LIBS} ${KDE4_KFILE_LIBS} ${KEXIV2_LIBRARIES} )
else (HAVE_KEXIV2)
    target_link_libraries(plasma_applet_frame ${KDE4_PLASMA_LIBS} ${KDE4_KIO_LIBS} ${KDE4_KFILE_LIBS} )
endif(HAVE_KEXIV2)

install(TARGETS plasma_applet_frame DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES plasma-frame-default.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES picture-frame-default.jpg DESTINATION ${DATA_INSTALL_DIR}/plasma-applet-frame/)
