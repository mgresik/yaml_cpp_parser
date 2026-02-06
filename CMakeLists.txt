cmake_minimum_required(VERSION 3.21)

project ("yaml_parser")

# Глобальные требования к компилятору
set(CMAKE_CXX_STANDARD          17 )
set(CMAKE_CXX_EXTENSIONS        OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON )

# В Windows + MS Visual Studio
if(MSVC)
    # Включение статической линковки
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    # Для удобной отладки
    set(CMAKE_VS_JUST_MY_CODE_DEBUGGING "$<$<CONFIG:Debug>:ON>")
endif()

# Настройка опций для библиотеки yaml-cpp
option(YAML_CPP_BUILD_TESTS off)
option(YAML_BUILD_SHARED_LIBS off)
option(YAML_CPP_INSTALL off)
option(YAML_CPP_BUILD_TOOLS off)

# Загрузка yaml-cpp с официального репозитория на GitHub
include(FetchContent)

FetchContent_Declare(
    yaml-cpp
    GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git
    GIT_TAG 47cd272
)
FetchContent_MakeAvailable(yaml-cpp)

# Добавление библиотеки, в которой реализовано чтение yaml-конфига
add_subdirectory(lib/yaml_config)

# Добавление основного demo-приложения
add_subdirectory(src)