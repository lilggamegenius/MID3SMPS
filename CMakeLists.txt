cmake_minimum_required(VERSION 3.5)
project(MID3SMPS)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include_directories(lib/imgui)
include_directories(lib/imgui/backends)
include_directories(lib/ImGuiFileDialog)

find_package(fmt REQUIRED)
#find_package(libremidi REQUIRED)
add_subdirectory(lib/libremidi)

include_directories(lib/libremidi/include)

set(imguiSrc
		lib/imgui/imgui.cpp
		lib/imgui/imgui_demo.cpp
		lib/imgui/imgui_draw.cpp
		lib/imgui/imgui_tables.cpp
		lib/imgui/imgui_widgets.cpp
		lib/imgui/misc/cpp/imgui_stdlib.cpp
		lib/ImGuiFileDialog/ImGuiFileDialog.cpp
		)

IF (CMAKE_SYSTEM_NAME STREQUAL "Windows" OR CMAKE_SYSTEM_NAME STREQUAL "Linux" OR CMAKE_SYSTEM_NAME STREQUAL "Darwin")
	find_package(PkgConfig REQUIRED)
	pkg_search_module(VULKAN REQUIRED vulkan)
	pkg_search_module(GLFW REQUIRED glfw3)
	include_directories(${GLFW_INCLUDE_DIRS})
	set(imguiBackendSrc
			lib/imgui/backends/imgui_impl_glfw.cpp
			lib/imgui/backends/imgui_impl_vulkan.cpp)
	set(DisplayAPI vulkan)
	set(DisplayAPILib ${VULKAN_LIBRARIES} ${GLFW_LIBRARIES})
ELSE ()
	find_package(PkgConfig REQUIRED)
	pkg_check_modules(SDL2 REQUIRED sdl2)
	include_directories(${SDL2_INCLUDE_DIRS})
	set(imguiBackendSrc
			lib/imgui/backends/imgui_impl_sdl.cpp
			lib/imgui/backends/imgui_impl_sdlrenderer.cpp)
	set(DisplayAPI sdl)
	set(DisplayAPILib ${SDL2_LIBRARIES})
ENDIF ()

set(src src/backend/${DisplayAPI}/WindowHandler.cpp src/windows/MainWindow.cpp src/windows/MainWindow.hpp)

add_executable(MID3SMPS ${imguiSrc} ${imguiBackendSrc} ${src})

target_link_libraries(MID3SMPS ${DisplayAPILib} fmt::fmt libremidi)
target_compile_definitions(MID3SMPS PUBLIC DisplayAPI=${DisplayAPI})
target_compile_options(MID3SMPS PUBLIC -Wall -Wextra -Wpedantic)