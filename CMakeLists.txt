cmake_minimum_required(VERSION 3.5)
project(MID3SMPS)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


find_package(fmt CONFIG REQUIRED)
find_package(gcem CONFIG REQUIRED)
#find_package(libremidi CONFIG REQUIRED)
add_subdirectory(lib/libremidi)

IF (CMAKE_SYSTEM_NAME STREQUAL "Windows" OR CMAKE_SYSTEM_NAME STREQUAL "Linux" OR CMAKE_SYSTEM_NAME STREQUAL "Darwin")
	find_package(PkgConfig REQUIRED)
	pkg_search_module(VULKAN REQUIRED vulkan)
	pkg_search_module(GLFW REQUIRED glfw3)
	include_directories(SYSTEM ${GLFW_INCLUDE_DIRS})
	set(imguiBackendSrc
			lib/imgui/backends/imgui_impl_glfw.cpp
			lib/imgui/backends/imgui_impl_vulkan.cpp)
	set(DisplayAPI vulkan)
	set(DisplayAPILib ${VULKAN_LIBRARIES} ${GLFW_LIBRARIES})
ELSE ()
	find_package(PkgConfig REQUIRED)
	pkg_check_modules(SDL2 REQUIRED sdl2)
	include_directories(SYSTEM ${SDL2_INCLUDE_DIRS})
	set(imguiBackendSrc
			lib/imgui/backends/imgui_impl_sdl.cpp
			lib/imgui/backends/imgui_impl_sdlrenderer.cpp)
	set(DisplayAPI sdl)
	set(DisplayAPILib ${SDL2_LIBRARIES})
ENDIF ()

set(src src/backend/${DisplayAPI}/WindowHandler.cpp src/windows/MainWindow.cpp src/windows/MainWindow.hpp)

set(imguiSrc
		lib/imgui/imgui.cpp
		lib/imgui/imgui_demo.cpp
		lib/imgui/imgui_draw.cpp
		lib/imgui/imgui_tables.cpp
		lib/imgui/imgui_widgets.cpp
		lib/imgui/misc/cpp/imgui_stdlib.cpp
		lib/ImGuiFileDialog/ImGuiFileDialog.cpp
)

if(MSVC)
	# Force to always compile with W4
	set(WARNING_FLAGS
			/W4
	)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
	set(WARNING_FLAGS
			#-Werror    # Turns all warnings into errors
			-Wall   # Enable most warning messages.
			-Wextra # Print extra (possibly unwanted) warnings.
			-Wpedantic # Issue warnings needed for strict compliance to the standard.
			-Wcast-align    # Warn about pointer casts which increase alignment. For example, warn if a char * is cast to an int * on machines where integers can only be accessed at two- or four-byte boundaries.
			-Wcast-qual # Warn about casts which discard qualifiers. For example, warn if a const char * is cast to an ordinary char *
			-Wctor-dtor-privacy # Warn when all constructors and destructors are private.
			-Wdisabled-optimization # Warn when an optimization pass is disabled.
			-Wformat=2  # Warn about printf/scanf/strftime/strfmon format string anomalies.
			-Winit-self # Warn about variables which are initialized to themselves.
			-Wlogical-op    # Warn when a logical operator is suspiciously always evaluating to true or false.
			-Wmissing-declarations # Warn about global functions without previous declarations.
			-Wmissing-include-dirs # Warn about user-specified include directories that do not exist.
			-Wnoexcept  # Warn when a noexcept expression evaluates to false even though the expression can't actually throw.
			-Wold-style-cast    # Warn if a C-style cast is used in a program.
			-Woverloaded-virtual    # Warn about overloaded virtual function names.
			-Wredundant-decls   # Warn about multiple declarations of the same object.
			-Wshadow    # Warn when one variable shadows another # Might disable this one
			-Wsign-conversion   # Warn for implicit type conversions between signed and unsigned integers.
			-Wsign-promo    # Warn when overload promotes from unsigned to signed.
			-Wstrict-null-sentinel  # Warn about un-casted NULL used as sentinel.
			-Wstrict-overflow=5 # Warn about optimizations that assume that signed overflow is undefined.
			-Wswitch-default    # Warn about enumerated switches missing a "default:" statement.
			-Wundef # Warn if an undefined macro is used in an #if directive.
			-Wzero-as-null-pointer-constant # Warn when a literal '0' is used as null pointer.
			-Wuseless-cast  # Warn about useless casts.

			#-Wno-unknown-pragmas # Disables warning about unknown pragmas since both clion and clang-tidy use their own
	)
else()
	message(WARNING "CMake flags for compiler aren't set for compiler ${CMAKE_CXX_COMPILER_ID}")
endif()

option(FORCE_COLORED_OUTPUT "Always produce ANSI-colored output (GNU/Clang only)." TRUE)
if(${FORCE_COLORED_OUTPUT})
	if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
		add_compile_options(-fdiagnostics-color=always)
	elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
		add_compile_options(-fcolor-diagnostics)
	endif()
endif()

add_library(ImGUI ${imguiSrc} ${imguiBackendSrc})
target_link_libraries(ImGUI ${DisplayAPILib})

include_directories(SYSTEM lib/imgui lib/imgui/backends lib/ImGuiFileDialog lib/libremidi/include)
include_directories(src)

add_executable(MID3SMPS ${src})
target_compile_options(MID3SMPS PUBLIC ${WARNING_FLAGS}) # Applies to all sources
target_link_libraries(MID3SMPS ImGUI fmt::fmt-header-only libremidi gcem)
target_compile_definitions(MID3SMPS
		PRIVATE
		# If the debug configuration pass the DEBUG define to the compiler
		$<$<CONFIG:Debug>:DEBUG>
)
#target_compile_definitions(MID3SMPS PUBLIC DisplayAPI=${DisplayAPI})