cmake_minimum_required(VERSION 3.25)
project(VulkanBootstrap VERSION 0.0.1)

add_subdirectory(external/tlsf)
add_subdirectory(external/spirv_reflect)

set(CMAKE_CXX_STANDARD 20)

find_package(volk REQUIRED)
find_package(spdlog REQUIRED)
find_package(glfw3)
find_package(glm)
find_package(stb)
find_package(cpptrace)
find_package(imgui)

file(GLOB_RECURSE HPP_SOURCES ${CMAKE_CURRENT_LIST_DIR}/include/*.hpp ${CMAKE_CURRENT_LIST_DIR}/external/include/*.*)
file(GLOB_RECURSE CPP_SOURCES ${CMAKE_CURRENT_LIST_DIR}/src/*.cpp)

message(STATUS "vulkan path $ENV{VULKAN_SDK}")

include_directories(${CMAKE_CURRENT_LIST_DIR}/external/include)
include_directories(${CMAKE_CURRENT_LIST_DIR}/include)

add_compile_definitions(PROJECT_VERSION="${CMAKE_PROJECT_VERSION}")
add_compile_definitions(GLM_FORCE_SWIZZLE)
add_compile_definitions(GLM_FORCE_DEPTH_ZERO_TO_ONE)
add_compile_definitions(GLM_FORCE_XYZW_ONLY)

add_executable(VulkanBootstrap ${HPP_SOURCES} ${CPP_SOURCES})
target_link_libraries(VulkanBootstrap
        spirv_reflect
        tlsf
        spdlog::spdlog
        volk::volk
        glfw
        glm::glm
        stb::stb
        cpptrace::cpptrace
        imgui::imgui)
