# Библиотека с функционалом работы с yaml-конфигом
add_library(yaml_config STATIC)

target_include_directories(yaml_config
    PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
        "$<INSTALL_INTERFACE:include>"
)

target_sources(yaml_config
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/include/demo_config_manager.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/include/demo_config.h"
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/src/demo_config_manager.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/demo_config.cpp"
)

target_link_libraries(yaml_config PRIVATE yaml-cpp)