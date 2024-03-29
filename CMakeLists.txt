cmake_minimum_required(VERSION 3.26)
project(MID3SMPS)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_UNITY_BUILD TRUE)

find_package(fmt CONFIG REQUIRED)
find_package(gcem CONFIG REQUIRED)
#find_package(libremidi CONFIG REQUIRED)
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	set_directory_properties(PROPERTIES COMPILE_DEFINITIONS _GLIBCXX_DEBUG)
endif ()
add_subdirectory(lib/libremidi)

set(IMGUIWRAP_CXX_STANDARD 23)
add_subdirectory(lib/imguiwrap)

set_target_properties(imgui PROPERTIES UNITY_BUILD OFF)

add_library(ImGuiFileDialog lib/ImGuiFileDialog/ImGuiFileDialog.cpp)
target_include_directories(ImGuiFileDialog SYSTEM PRIVATE lib/imguiwrap/vendor/imgui/src)
target_compile_options(ImGuiFileDialog PRIVATE "-w") # Disable warnings since its not our project to maintain

if(MSVC)
	# Force to always compile with W4
	set(WARNING_FLAGS
			/W4
	)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
	set(WARNING_FLAGS
			-Werror    # Turns all warnings into errors
			-Wall   # Enable most warning messages.
			-Wextra # Print extra (possibly unwanted) warnings.
			-Wpedantic # Issue warnings needed for strict compliance to the standard.
			-Wcast-align    # Warn about pointer casts which increase alignment. For example, warn if a char * is cast to an int * on machines where integers can only be accessed at two- or four-byte boundaries.
			-Wcast-qual # Warn about casts which discard qualifiers. For example, warn if a const char * is cast to an ordinary char *
			-Wctor-dtor-privacy # Warn when all constructors and destructors are private.
			-Wdisabled-optimization # Warn when an optimization pass is disabled.
			-Wformat=2  # Warn about printf/scanf/strftime/strfmon format string anomalies.
			-Winit-self # Warn about variables which are initialized to themselves.
			-Wmissing-declarations # Warn about global functions without previous declarations.
			-Wmissing-include-dirs # Warn about user-specified include directories that do not exist.
			-Wold-style-cast    # Warn if a C-style cast is used in a program.
			-Woverloaded-virtual    # Warn about overloaded virtual function names.
			-Wredundant-decls   # Warn about multiple declarations of the same object.
			-Wshadow    # Warn when one variable shadows another # Might disable this one
			-Wsign-conversion   # Warn for implicit type conversions between signed and unsigned integers.
			-Wsign-promo    # Warn when overload promotes from unsigned to signed.
			-Wstrict-overflow=5 # Warn about optimizations that assume that signed overflow is undefined.
			-Wswitch-default    # Warn about enumerated switches missing a "default:" statement.
			-Wundef # Warn if an undefined macro is used in an #if directive.
			-Wzero-as-null-pointer-constant # Warn when a literal '0' is used as null pointer.

			#-Wno-unknown-pragmas # Disables warning about unknown pragmas since both clion and clang-tidy use their own

			-Wno-error=missing-include-dirs # One of the libraries is causing this warning to show up but IDK which
	)
	if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
		set(WARNING_FLAGS ${WARNING_FLAGS}
				#-fhardened
				-D_FORTIFY_SOURCE=3
				-D_GLIBCXX_ASSERTIONS
				-ftrivial-auto-var-init=zero
				-fPIE  -pie  -Wl,-z,relro,-z,now-
				-fstack-protector-strong
				-fstack-clash-protection

				-Wlogical-op    # Warn when a logical operator is suspiciously always evaluating to true or false.
				-Wnoexcept  # Warn when a noexcept expression evaluates to false even though the expression can't actually throw.
				-Wstrict-null-sentinel  # Warn about un-casted NULL used as sentinel.
				-Wuseless-cast  # Warn about useless casts.
		)
	endif()
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

add_library(MID3SMPS STATIC
		src/backend/window_handler.cpp
		src/windows/window.hpp
		src/windows/main_window.cpp src/windows/main_window.hpp
		src/windows/ym2612_edit.cpp src/windows/ym2612_edit.hpp
		src/containers/program_persistence.cpp src/containers/program_persistence.hpp
		src/containers/files/mapping.cpp src/containers/files/mapping.hpp
		src/containers/files/formatException.hpp
		src/containers/files/gyb.cpp src/containers/files/gyb.hpp
		src/containers/files/fm/patch.cpp src/containers/files/FM/patch.hpp
		src/containers/files/fm/operators.hpp
)

target_include_directories(MID3SMPS SYSTEM PRIVATE lib/ImGuiFileDialog)

target_include_directories(MID3SMPS PUBLIC src)
target_compile_options(MID3SMPS PUBLIC ${WARNING_FLAGS})
target_link_libraries(MID3SMPS imguiwrap ImGuiFileDialog fmt::fmt-header-only libremidi gcem)
target_compile_definitions(MID3SMPS
		PUBLIC
		# If the debug configuration pass the DEBUG define to the compiler
		$<$<CONFIG:Debug>:DEBUG _GLIBCXX_DEBUG>
)

add_executable(MID3SMPS_EXECUTABLE
		src/entry.cpp
)

target_link_libraries(MID3SMPS_EXECUTABLE MID3SMPS)

set(FONTS_SRC data/fonts)
set(FONTS_DST ${FONTS_SRC})

file(INSTALL ${FONTS_SRC}/SourceCodePro-Black.ttf ${FONTS_SRC}/SourceCodePro-Semibold.ttf DESTINATION ${CMAKE_BINARY_DIR}/${FONTS_DST})

add_subdirectory(test)