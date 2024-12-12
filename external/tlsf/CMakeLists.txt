cmake_minimum_required(VERSION 3.25)
project(tlsf VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 20)

set(TLSF_INCLUDE_DIR $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/.>)


add_library(tlsf tlsf.c tlsf.h)
target_include_directories(tlsf PUBLIC  ${TLSF_INCLUDE_DIR})