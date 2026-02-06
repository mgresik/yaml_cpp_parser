add_executable(yaml_parser)

target_sources(yaml_parser
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp"
)

target_link_libraries(yaml_parser PRIVATE yaml_config)

# Копирование config.yml в директорию сборки к исполняемому файлу
# для удобства отладки и экспериментов
configure_file(
    "${CMAKE_SOURCE_DIR}/config.yml"
    "${CMAKE_CURRENT_BINARY_DIR}/config.yml"
    COPYONLY
)