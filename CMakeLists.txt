cmake_minimum_required(VERSION 3.25)
project(VulkanBootstrap VERSION 0.0.1)

add_subdirectory(external/tlsf)
add_subdirectory(external/spirv_reflect)

set(CMAKE_CXX_STANDARD 20)

find_package(volk REQUIRED)
find_package(spdlog REQUIRED)
find_package(fmt)
find_package(glfw3)
find_package(glm)
find_package(stb)
find_package(cpptrace)
find_package(imgui)

find_program(GLSLC_EXECUTABLE
        NAMES glslc
        HINTS "$ENV{VULKAN_SDK}/Bin" "$ENV{VULKAN_SDK}/bin"
        REQUIRED)

file(GLOB_RECURSE HPP_SOURCES ${CMAKE_CURRENT_LIST_DIR}/include/*.hpp ${CMAKE_CURRENT_LIST_DIR}/external/include/*.*)
file(GLOB_RECURSE CPP_SOURCES ${CMAKE_CURRENT_LIST_DIR}/src/*.cpp)
file(GLOB SHADER_SOURCES CONFIGURE_DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/resources/shaders/*.vert
        ${CMAKE_CURRENT_SOURCE_DIR}/resources/shaders/*.frag
        ${CMAKE_CURRENT_SOURCE_DIR}/resources/shaders/*.comp)

message(STATUS "vulkan path $ENV{VULKAN_SDK}")

include_directories(${CMAKE_CURRENT_LIST_DIR}/external/include)
include_directories(${CMAKE_CURRENT_LIST_DIR}/include)

add_compile_definitions(PROJECT_VERSION="${CMAKE_PROJECT_VERSION}")
add_compile_definitions(GLM_FORCE_SWIZZLE)
add_compile_definitions(GLM_FORCE_DEPTH_ZERO_TO_ONE)
add_compile_definitions(GLM_FORCE_XYZW_ONLY)

set(COMPILED_SHADER_OUTPUTS)
foreach(SHADER_SOURCE IN LISTS SHADER_SOURCES)
    get_filename_component(SHADER_NAME ${SHADER_SOURCE} NAME)
    set(SHADER_OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/resources/shaders/${SHADER_NAME}.spv)
    list(APPEND COMPILED_SHADER_OUTPUTS ${SHADER_OUTPUT})

    add_custom_command(
            OUTPUT ${SHADER_OUTPUT}
            COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/resources/shaders
            COMMAND ${GLSLC_EXECUTABLE} ${SHADER_SOURCE} -o ${SHADER_OUTPUT}
            DEPENDS ${SHADER_SOURCE}
            COMMENT "Compiling shader ${SHADER_NAME}"
            VERBATIM)
endforeach()

add_custom_target(compile_shaders ALL DEPENDS ${COMPILED_SHADER_OUTPUTS})

add_executable(VulkanBootstrap ${HPP_SOURCES} ${CPP_SOURCES})
add_dependencies(VulkanBootstrap compile_shaders)
target_link_libraries(VulkanBootstrap
        spirv_reflect
        tlsf
        fmt::fmt
        spdlog::spdlog
        volk::volk
        glfw
        glm::glm
        stb::stb
        cpptrace::cpptrace
        imgui::imgui)

add_custom_command(TARGET VulkanBootstrap POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/resources/images
        $<TARGET_FILE_DIR:VulkanBootstrap>/resources/images
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_BINARY_DIR}/resources/shaders
        $<TARGET_FILE_DIR:VulkanBootstrap>/resources/shaders
        COMMENT "Copying runtime resources to output directory")
