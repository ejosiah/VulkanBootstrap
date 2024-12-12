cmake_minimum_required(VERSION 3.25)
project(spirv_reflect VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 20)

set(SPIRV_INCLUDE_DIR $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/.>)


add_library(spirv_reflect spirv.h spirv_reflect.h spirv_reflect.cpp)
target_include_directories(spirv_reflect PUBLIC  ${SPIRV_INCLUDE_DIR})