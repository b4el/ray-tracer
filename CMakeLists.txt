cmake_minimum_required(VERSION 3.16)
project(RayTracer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Ray tracing faz MUITA conta: compile em Release para ficar ~10x mais rapido
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release)
endif()

if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(Threads REQUIRED)

# Nucleo do ray tracer (usado pelo programa e pelos testes)
add_library(raytracer_core
    src/BVH.cpp
    src/Camera.cpp
    src/Cena.cpp
    src/Cenas.cpp
    src/Esfera.cpp
    src/Imagem.cpp
    src/Malha.cpp
    src/Material.cpp
    src/Renderizador.cpp
    src/Triangulo.cpp
)
target_include_directories(raytracer_core PUBLIC include)
target_link_libraries(raytracer_core PUBLIC Threads::Threads)

add_executable(raytracer src/main.cpp)
target_link_libraries(raytracer PRIVATE raytracer_core)

# Copia os modelos 3D para perto do executavel (o Visual Studio roda o programa dali)
add_custom_command(TARGET raytracer POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/modelos
            $<TARGET_FILE_DIR:raytracer>/modelos
)

# Testes
enable_testing()
add_executable(testes tests/testes.cpp)
target_link_libraries(testes PRIVATE raytracer_core)
add_test(NAME testes_raytracer COMMAND testes)

# Etapas do tutorial (cada uma e um programa independente)
file(GLOB ETAPAS CONFIGURE_DEPENDS tutorial/etapa*.cpp)
foreach(arquivo ${ETAPAS})
    get_filename_component(nome ${arquivo} NAME_WE)
    add_executable(${nome} ${arquivo})
    target_include_directories(${nome} PRIVATE tutorial)
endforeach()
