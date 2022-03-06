add_library(engine_dict_static STATIC dictengine.cpp)

target_link_libraries (engine_dict_static
  KF5::I18n
  Qt::Network
)

