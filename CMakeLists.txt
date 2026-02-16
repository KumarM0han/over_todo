cmake_minimum_required(VERSION 3.24)

project(mytodoapp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build types (default to Debug for single-config generators)
if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Choose the type of build." FORCE)
endif()
if(NOT CMAKE_CONFIGURATION_TYPES)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release")
endif()

# Help CMake find packages if VULKAN_SDK is installed in a custom location (e.g. C:\VulkanSDK\1.4.341.1)
if(DEFINED ENV{VULKAN_SDK})
	list(APPEND CMAKE_PREFIX_PATH $ENV{VULKAN_SDK})
	list(APPEND CMAKE_PREFIX_PATH "$ENV{VULKAN_SDK}/Lib/cmake")
endif()

find_package(Vulkan REQUIRED)
find_package(SDL3 REQUIRED CONFIG)

set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/vendored/imgui)

set(SOURCES
	src/main.cpp
	src/Todo.cpp
	src/Log.cpp
	src/Utils.cpp
	src/imgui_demo.cpp
	src/imgui_draw.cpp
	${IMGUI_DIR}/imgui.cpp
	${IMGUI_DIR}/imgui_tables.cpp
	${IMGUI_DIR}/imgui_widgets.cpp
	${IMGUI_DIR}/imgui_impl_sdl3.cpp
	${IMGUI_DIR}/imgui_impl_vulkan.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME}
	PRIVATE
		${IMGUI_DIR}
		${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(${PROJECT_NAME}
	PRIVATE
		Vulkan::Vulkan
		SDL3::SDL3
)

if(MSVC)
	target_compile_options(${PROJECT_NAME} PRIVATE /W4 /permissive-)
else()
	target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
endif()


set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
