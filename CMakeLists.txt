cmake_minimum_required(VERSION 3.14)
project(orbit VERSION 1.0.0 LANGUAGES CXX)

add_library(orbit INTERFACE)
add_library(orbit::orbit ALIAS orbit)
target_include_directories(orbit INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include> $<INSTALL_INTERFACE:include>)
target_compile_features(orbit INTERFACE cxx_std_20)