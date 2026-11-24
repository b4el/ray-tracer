cmake_minimum_required(VERSION 3.16)
project(RayTracer LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Ray tracing faz MUITA conta: compile em Release para ficar ~10x mais rapido
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Avisos ligados so no NOSSO codigo (nao no codigo do Lua)
if(MSVC)
    set(AVISOS /W4)
else()
    set(AVISOS -Wall -Wextra -Wpedantic)
endif()

find_package(Threads REQUIRED)

# ---------- Lua 5.4 (baixado automaticamente na primeira configuracao) ----------
# O repositorio do Lua nao tem CMakeLists.txt, entao o FetchContent so baixa os
# fontes e nos mesmos montamos a biblioteca logo abaixo.
include(FetchContent)
FetchContent_Declare(lua
    GIT_REPOSITORY https://github.com/lua/lua.git
    GIT_TAG        v5.4.7
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(lua)

file(GLOB LUA_FONTES ${lua_SOURCE_DIR}/l*.c)
list(REMOVE_ITEM LUA_FONTES ${lua_SOURCE_DIR}/lua.c ${lua_SOURCE_DIR}/ltests.c)  # interpretador e testes internos
add_library(lua_static STATIC ${LUA_FONTES})
target_include_directories(lua_static PUBLIC ${lua_SOURCE_DIR})
if(MSVC)
    target_compile_options(lua_static PRIVATE /W0)
    target_compile_definitions(lua_static PRIVATE _CRT_SECURE_NO_WARNINGS)
else()
    target_compile_options(lua_static PRIVATE -w)
    target_compile_definitions(lua_static PRIVATE LUA_USE_POSIX)
    target_link_libraries(lua_static PUBLIC m)
endif()

# ---------- Nucleo do ray tracer (usado pelo programa e pelos testes) ----------
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
    src/ScriptLua.cpp
    src/Triangulo.cpp
)
target_include_directories(raytracer_core PUBLIC include)
target_link_libraries(raytracer_core PUBLIC Threads::Threads PRIVATE lua_static)
target_compile_options(raytracer_core PRIVATE ${AVISOS})

add_executable(raytracer src/main.cpp)
target_link_libraries(raytracer PRIVATE raytracer_core)
target_compile_options(raytracer PRIVATE ${AVISOS})

# Copia os modelos 3D e as cenas em Lua para perto do executavel
# (o Visual Studio roda o programa a partir dessa pasta). Roda em TODO build,
# para que uma cena .lua editada seja copiada mesmo sem recompilar o C++.
if(CMAKE_CONFIGURATION_TYPES)            # geradores com varias configuracoes (ex.: solucao .sln)
    set(PASTA_DO_EXE ${CMAKE_BINARY_DIR}/$<CONFIG>)
else()                                   # Ninja / Makefiles (o padrao do Visual Studio com CMake)
    set(PASTA_DO_EXE ${CMAKE_BINARY_DIR})
endif()
add_custom_target(copiar_recursos ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/modelos ${PASTA_DO_EXE}/modelos
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/cenas ${PASTA_DO_EXE}/cenas
    COMMENT "Copiando modelos/ e cenas/"
)
add_dependencies(raytracer copiar_recursos)

# ---------- Testes ----------
enable_testing()
add_executable(testes tests/testes.cpp)
target_link_libraries(testes PRIVATE raytracer_core)
target_compile_options(testes PRIVATE ${AVISOS})
add_test(NAME testes_raytracer COMMAND testes)

# ---------- Etapas do tutorial (cada uma e um programa independente) ----------
file(GLOB ETAPAS CONFIGURE_DEPENDS tutorial/etapa*.cpp)
foreach(arquivo ${ETAPAS})
    get_filename_component(nome ${arquivo} NAME_WE)
    add_executable(${nome} ${arquivo})
    target_include_directories(${nome} PRIVATE tutorial)
    target_compile_options(${nome} PRIVATE ${AVISOS})
endforeach()
