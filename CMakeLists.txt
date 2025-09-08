cmake_minimum_required(VERSION 3.20)
project(pathtracer LANGUAGES CXX C)

# Choose your standard (the code is C++11-friendly; C++17 also fine)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(USE_OPENMP "Enable OpenMP parallel rendering" ON)

# Sources
set(SRC
    src/main.cpp
    src/material.cpp
    src/objects.cpp
    src/kdtree.cpp
    src/texture.cpp
    src/camera.cpp
    src/scene.cpp
    src/renderer.cpp
    lib/rand48/rand48.c
    lib/lodepng/lodepng.cpp
    lib/tiny_obj_loader/tiny_obj_loader.cc
)

add_executable(pathtracer ${SRC})

target_include_directories(pathtracer PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/lodepng
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/rand48
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/tiny_obj_loader
)

# Reasonable, portable optimizations (no -static/-m64/-march=native)
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|AppleClang|GNU")
  target_compile_options(pathtracer PRIVATE -O3 -ffast-math -funroll-loops)
endif()

# Make asset paths robust (usable from build dir)
target_compile_definitions(pathtracer PRIVATE
  ASSET_DIR="${CMAKE_CURRENT_SOURCE_DIR}/obj"
)

# OpenMP (optional). On macOS prefer Homebrew LLVM or libomp.
if (USE_OPENMP)
  find_package(OpenMP)
  if (OpenMP_CXX_FOUND)
    target_link_libraries(pathtracer PRIVATE OpenMP::OpenMP_CXX)
    target_compile_definitions(pathtracer PRIVATE USE_OPENMP=1)
  else()
    message(WARNING "OpenMP requested but not found; building without it.")
  endif()
endif()