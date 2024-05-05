cmake_minimum_required(VERSION 3.26)
project(MID3SMPS)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_UNITY_BUILD FALSE)

find_package(fmt CONFIG REQUIRED)
find_package(gcem CONFIG REQUIRED)
#find_package(libremidi CONFIG REQUIRED)
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
	set_directory_properties(PROPERTIES COMPILE_DEFINITIONS _GLIBCXX_DEBUG)
endif ()
add_subdirectory(lib/libremidi)

set(IMGUIWRAP_CXX_STANDARD 23)
add_subdirectory(lib/imguiwrap)

set_target_properties(imgui PROPERTIES UNITY_BUILD OFF)

add_library(ImGuiFileDialog lib/ImGuiFileDialog/ImGuiFileDialog.cpp)
target_include_directories(ImGuiFileDialog SYSTEM PRIVATE lib/imguiwrap/vendor/imgui/src)

add_library(Nuked_OPN2 lib/Nuked-OPN2/ym3438.c)
target_include_directories(Nuked_OPN2 SYSTEM PRIVATE lib/Nuked-OPN2)

if (MSVC)
	# Force to always compile with W4
	set(WARNING_FLAGS
			/Wall
			#/WX
			/wd5030 # Disable warnings from unrecognized attributes
			/wd4820 # Disable warnings for struct padding


			/Zc:static_assert- # Allow static_assert(false) when used in templates in untaken codepaths # CWG2518
			/external:W0 # Disable warnings in library headers
			/external:anglebrackets # Treat all headers in angle brackets as external
	)
	set(OPTIMIZATION_FLAGS
			$<$<NOT:$<CONFIG:Debug>>:/O2>
	)
	set(EXTERN_WARNING_FLAGS
			/W0
	)
	add_compile_definitions(__WIN32) # This is supposed to be defined by MSVC, but its not for some reason
	add_compile_definitions(IMGUI_DISABLE_WIN32_FUNCTIONS)
elseif (CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
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
			-Wconversion    # Ditto
			-Wsign-promo    # Warn when overload promotes from unsigned to signed.
			-Wstrict-overflow=2 # Warn about optimizations that assume that signed overflow is undefined.
			-Wswitch-default    # Warn about enumerated switches missing a "default:" statement.
			-Wundef # Warn if an undefined macro is used in an #if directive.
			-Wzero-as-null-pointer-constant # Warn when a literal '0' is used as null pointer.

			#-Wno-unknown-pragmas # Disables warning about unknown pragmas since both clion and clang-tidy use their own

			-Wno-error=missing-include-dirs # One of the libraries is causing this warning to show up but IDK which
	)
	if (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
		set(WARNING_FLAGS ${WARNING_FLAGS}
			#-fhardened
			#-D_FORTIFY_SOURCE=3
			-D_GLIBCXX_DEBUG
			#-ftrivial-auto-var-init=zero
			-fPIE -pie -Wl,-z,relro,-z,now-
			-fstack-protector-strong
			-fstack-clash-protection

			-Wlogical-op    # Warn when a logical operator is suspiciously always evaluating to true or false.
			-Wnoexcept  # Warn when a noexcept expression evaluates to false even though the expression can't actually throw.
			-Wstrict-null-sentinel  # Warn about un-casted NULL used as sentinel.
			-Wuseless-cast  # Warn about useless casts.
		)
	else () # Clang
		set(WARNING_FLAGS ${WARNING_FLAGS}
				-Wno-error=unknown-attributes # clang 18 doesn't support [[assume()]] yet, so its considered unknown
		)
	endif ()
	set(OPTIMIZATION_FLAGS
			$<IF:$<CONFIG:Debug>,-Og,-Ofast>
	)
	set(EXTERN_WARNING_FLAGS
			-w
	)
else ()
	message(WARNING "CMake flags for compiler aren't set for compiler ${CMAKE_CXX_COMPILER_ID}")
endif ()

target_compile_options(ImGuiFileDialog PRIVATE ${EXTERN_WARNING_FLAGS}) # Disable warnings since its not our project to maintain
target_compile_options(Nuked_OPN2 PRIVATE ${EXTERN_WARNING_FLAGS})

add_compile_options(${OPTIMIZATION_FLAGS})

option(FORCE_COLORED_OUTPUT "Always produce ANSI-colored output (GNU/Clang only)." TRUE)
if (${FORCE_COLORED_OUTPUT})
	if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
		add_compile_options(-fdiagnostics-color=always)
	elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
		add_compile_options(-fcolor-diagnostics)
	endif ()
endif ()

add_library(MID3SMPS STATIC
		src/gui/backend/window_handler.cpp

		src/gui/windows/window.hpp
		src/gui/windows/main_window.cpp src/gui/windows/main_window.hpp
		src/gui/windows/ym2612_edit.cpp src/gui/windows/ym2612_edit.hpp

		src/containers/program_persistence.cpp src/containers/program_persistence.hpp

		src/containers/instrument.hpp
		src/containers/fm_instrument.hpp
		src/containers/instrument_bank.cpp src/containers/instrument_bank.hpp

		src/containers/files/mid2smps/mapping.cpp src/containers/files/mid2smps/mapping.hpp
		src/containers/files/mid2smps/gyb.cpp src/containers/files/mid2smps/gyb.hpp
		src/containers/files/mid2smps/fm/patch.cpp src/containers/files/mid2smps/fm/patch.hpp

		src/containers/chips/ym2612/operators.hpp

		src/helpers/safe_int.hpp
		src/helpers/default_usings.hpp
		src/helpers/list_helper.hpp

		src/exceptions/formatException.hpp
)

target_include_directories(MID3SMPS SYSTEM PRIVATE lib/ImGuiFileDialog)

target_include_directories(MID3SMPS PUBLIC src)
target_compile_options(MID3SMPS PUBLIC ${WARNING_FLAGS})
target_link_libraries(MID3SMPS imguiwrap ImGuiFileDialog fmt::fmt-header-only libremidi gcem Nuked_OPN2)
target_compile_definitions(MID3SMPS
		PUBLIC
		# If the debug configuration pass the DEBUG define to the compiler
		$<$<CONFIG:Debug>:DEBUG>
)

add_executable(MID3SMPS_EXECUTABLE
		src/entry.cpp
)

target_link_libraries(MID3SMPS_EXECUTABLE MID3SMPS)

set(FONTS_SRC data/fonts)
set(FONTS_DST ${FONTS_SRC})

add_custom_command(TARGET MID3SMPS_EXECUTABLE POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:MID3SMPS_EXECUTABLE>/${FONTS_DST}
		COMMAND ${CMAKE_COMMAND} -E copy_if_different
		${FONTS_SRC}/SourceCodePro-Black.ttf ${FONTS_SRC}/SourceCodePro-Semibold.ttf
		$<TARGET_FILE_DIR:MID3SMPS_EXECUTABLE>/${FONTS_DST})

add_subdirectory(test)