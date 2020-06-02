cmake_minimum_required(VERSION 3.16)

project(engine LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(CheckCXXCompilerFlag)
include(CheckCCompilerFlag)
set(ENGINE_EXTRA_FLAGS "")
macro(add_supported_c_flags flag)
	string(REPLACE "=" "__" var_name support_c_${flag})
	check_c_compiler_flag(${flag} ${var_name})
	if (${var_name})
		set(ENGINE_EXTRA_FLAGS ${ENGINE_EXTRA_FLAGS} ${flag})
	endif()
endmacro()

macro(add_supported_cxx_flags flag)
	string(REPLACE "=" "__" var_name engine_cxx_flag_${flag})
	check_cxx_compiler_flag(${flag} ${var_name})
	if (${var_name})
		set(ENGINE_EXTRA_FLAGS ${ENGINE_EXTRA_FLAGS} ${flag})
	else()
		add_supported_c_flags(${flag})
	endif()
endmacro()

## These two things are needed for YCM
SET( CMAKE_EXPORT_COMPILE_COMMANDS ON )
IF( EXISTS "${CMAKE_CURRENT_BINARY_DIR}/compile_commands.json" )
  EXECUTE_PROCESS( COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${CMAKE_CURRENT_BINARY_DIR}/compile_commands.json
    ${CMAKE_CURRENT_SOURCE_DIR}/compile_commands.json
  )
ENDIF()

function(set_option_if_not_set var_name desc default)
	if (NOT DEFINED ${var_name})
		set(init_val ${default})
	else()
		set(init_val ${${var_name}})
	endif()
	message("ENGINE option: ${var_name} ${init_val}")
	option(${var_name} ${desc} ${init_val})
endfunction(set_option_if_not_set)

set_option_if_not_set(ENGINE_ENABLE_AUDIO "Enable Audio Playback" ON)
if (APPLE)
	set_option_if_not_set(ENGINE_ENABLE_VR "Enable VR Support" OFF)
else()
	set_option_if_not_set(ENGINE_ENABLE_VR "Enable VR Support" ON)
endif()
set_option_if_not_set(ENGINE_ENABLE_TEXT "Enable text rendering" ON)
set_option_if_not_set(ENGINE_ENABLE_ANIMATION "Enable animation library" ON)
set_option_if_not_set(ENGINE_ENABLE_JSON "Enable JSON decoding" ON)
set_option_if_not_set(ENGINE_MIN_GAME_OBJECT "Switch to minimum-game-object class" OFF)
set_option_if_not_set(ENGINE_ENABLE_MULTITHREADED "Allow multithreaded engine, may impact performance" OFF)

set_option_if_not_set(ENGINE_EXTRA_COMPILER_CHECKS "Enable more strict compiler checks" ON)
set_option_if_not_set(ENGINE_WERROR "Enable -WError (enabled in CI)" OFF)

set_option_if_not_set(ENGINE_FETCH_CONTENT_DISABLE_UPDATES "Enable Running Updates for dependences. Here for faster local running" OFF)
set_option_if_not_set(ENGINE_FETCH_CONTENT_QUIET "Use quiet mode for fetchContent" ON)

# TODO: Debug each component?
set_option_if_not_set(ENGINE_ENABLE_TESTING "Enable Unit Testing" ON)
set_option_if_not_set(ENGINE_DEBUG "Enable debug messages in engine" ON)
set_option_if_not_set(ENGINE_DEBUG_VR "Debug VR Components" ON)

set_option_if_not_set(ENGINE_CXX_OVERLOADS "Enable CXX-style overloads" ON)

if (APPLE AND ENGINE_ENABLE_VR)
	message(FATAL_ERROR "VR Not supported on MacOS.  Feel free to leave an issue, or submit a pull request.")
endif()

if(WIN32)
	set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif()

if (ENGINE_EXTRA_COMPILER_CHECKS)
	message("Enabling other compiler checks")
	if (MSVC)
		add_supported_cxx_flags("/Wall")
		add_supported_cxx_flags("/W3")
	else()
		# -pedantic -Wall -Wextra -Wundef -Wcast-align -Wchar-subscripts -Wnon-virtual-dtor -Wunused-local-typedefs -Wpointer-arith -Wwrite-strings -Wformat-security -Wlogical-op -Wdouble-promotion -Wshadow -Wno-psabi -Wno-variadic-macros -Wno-long-long -fno-check-new -fno-common -fstrict-aliasing -ansi
		add_supported_cxx_flags("-Wall")
		add_supported_cxx_flags("-Wextra") 
		add_supported_cxx_flags("-Wconversion")
		add_supported_cxx_flags("-Wunreachable-code")
		add_supported_cxx_flags("-Wuninitialized")
		add_supported_cxx_flags("-pedantic-errors")
		# add_supported_cxx_flags("-Wold-style-cast")
		add_supported_cxx_flags("-Wno-error=unused-variable")
		add_supported_cxx_flags("-Wshadow")
		add_supported_cxx_flags("-Wfloat-equal")
		add_supported_cxx_flags("-Wduplicated-cond")
		add_supported_cxx_flags("-Wno-error=duplicated-branches")
		add_supported_cxx_flags("-Wlogical-op")
		add_supported_cxx_flags("-Wrestrict")
		add_supported_cxx_flags("-Wnull-dereference")
		add_supported_cxx_flags("-Wuseless-cast")
		# add_supported_c_flags("-Wjump-misses-init") # C only
		add_supported_cxx_flags("-Wno-error=double-promotion")
		add_supported_cxx_flags("-Wformat=2")
		add_supported_cxx_flags("-Wformat-truncation")
		add_supported_cxx_flags("-Wformat-overflow")
		add_supported_cxx_flags("-Wshift-overflow")
		add_supported_cxx_flags("-Wundef")
		add_supported_cxx_flags("-fno-common")
		add_supported_cxx_flags("-Wswitch-enum")
		add_supported_cxx_flags("-Wno-error=effc++")
		add_supported_cxx_flags("-fanalyzer")
	endif()
endif()

if (ENGINE_WERROR)
	message("Enabling WError")
	if (MSVC)
		add_supported_cxx_flags("/WX")
	else()
		set(ENGINE_EXTRA_FLAGS ${ENGINE_EXTRA_FLAGS} -Werror)
	endif()
endif()
message("ENGINE: Extra flags: " ${ENGINE_EXTRA_FLAGS})

find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM AND NOT CMAKE_GENERATOR STREQUAL "Xcode" AND NOT ${ENGINE_WERROR})
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CCACHE_PROGRAM}")
endif()

# Include the addons
set(FETCHCONTENT_UPDATES_DISCONNECTED ${ENGINE_FETCH_CONTENT_DISABLE_UPDATES})
set(FETCHCONTENT_QUIET ${ENGINE_FETCH_CONTENT_QUIET})
add_subdirectory(cmake)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

if (ENGINE_ENABLE_TESTING)
	enable_testing()
	include(external/catch2.cmake)
endif()

file(GLOB SRC_FILES src/*.cpp)
file(GLOB INC_FILES include/engine/*.hpp)
add_library(engine ${SRC_FILES} ${INC_FILES})
include_directories(include)

find_package(OpenGL REQUIRED)
find_package(Threads REQUIRED)

include(external/glm.cmake)
include(external/stb_image.cmake)
include(external/glad.cmake)

find_package(GLFW3 REQUIRED)

fetch_extern(glslang https://github.com/KhronosGroup/glslang.git 8.13.3743)
get_property(glslang_BINARY_DIR GLOBAL PROPERTY glslang_BINARY_DIR)
function(test_shaders_func SHADERS end_target)
	add_custom_target(${end_target})
	set(${end_target}_DEPENDS "")

	# Get location of final binary
	set(glslangValidatorProg $<TARGET_FILE:glslangValidator>)

	foreach(shader IN LISTS SHADERS)
		message("${end_target} - Found shader: ${shader}")
		get_filename_component(shader_name ${shader} NAME)
		if (NOT TARGET "${end_target}_shader_${shader_name}")
			add_custom_target("${end_target}_shader_${shader_name}"
				COMMAND ${glslangValidatorProg} ${shader}
				DEPENDS glslangValidator ${shader}
				WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
				COMMENT "${end_target} - Verifying Shader: ${shader_name}")
			list(APPEND ${end_target}_DEPENDS "${end_target}_shader_${shader_name}")
		endif()
		add_dependencies(${end_target} ${${end_target}_DEPENDS})
	endforeach()
endfunction()

# VR Library depends on Eigen
include(external/eigen.cmake)

include(external/assimp.cmake)
if (WIN32)
    #ADD_CUSTOM_TARGET(EngineUpdateAssimpLibsDebugSymbolsAndDLLs COMMENT "Copying Assimp Libraries ..." VERBATIM)

	if (MSVC12)
		SET(ASSIMP_MSVC_VERSION "vc120")
    ELSEIF(MSVC14)
		SET(ASSIMP_MSVC_VERSION "vc141")
    ELSEIF(MSVC15)
		SET(ASSIMP_MSVC_VERSION "vc141")
    ENDIF()
	get_property(assimp_BINARY_DIR GLOBAL PROPERTY assimp_BINARY_DIR)
	add_custom_target(EngineUpdateAssimpLibsDebugSymbolsAndDLLs 
		COMMAND ${CMAKE_COMMAND} -E copy ${assimp_BINARY_DIR}/code/Debug/assimp-${ASSIMP_MSVC_VERSION}-mt.dll ${CMAKE_BINARY_DIR}/assimp-${ASSIMP_MSVC_VERSION}-mt.dll
		DEPENDS assimp::assimp
		VERBATIM)
	# file(COPY ${assimp_BINARY_DIR}/code/Debug/assimp-${ASSIMP_MSVC_VERSION}-mt.dll DESTINATION ${CMAKE_BINARY_DIR})
	add_dependencies(engine EngineUpdateAssimpLibsDebugSymbolsAndDLLs) # not working for now
endif()

get_target_property(assimp_INCLUDE_DIRS assimp::assimp INTERFACE_INCLUDE_DIRECTORIES)
target_include_directories(engine SYSTEM PUBLIC ${assimp_INCLUDE_DIRS})

add_subdirectory(components/constants)

if (ENGINE_ENABLE_AUDIO)
	include(external/openal.cmake)
	target_compile_definitions(engine PUBLIC "-DENGINE_ENABLE_AUDIO=1")
else()
	target_compile_definitions(engine PUBLIC "-DENGINE_ENABLE_AUDIO=0")
endif()

if (ENGINE_ENABLE_VR)
	# Fetch OpenVR
	include(external/openvr.cmake)
	get_property(openvr_SOURCE_DIR GLOBAL PROPERTY openvr_SOURCE_DIR)
	target_include_directories(engine SYSTEM PRIVATE ${openvr_SOURCE_DIR}/headers)
	if (WIN32)
		set(OPENVR_LIBRARY openvr_api64)
	else()
		set(OPENVR_LIBRARY openvr_api)
	endif()
	target_compile_definitions(engine PUBLIC "-DENGINE_ENABLE_VR=1")

	# Add all platforms, will include VR
	add_subdirectory(components/platforms)

	set(PRIV_VR_LIB vr)
else()
	target_compile_definitions(engine PUBLIC "-DENGINE_ENABLE_VR=0")
endif()

if (ENGINE_ENABLE_TEXT)
	include(external/freetype.cmake)
	target_compile_definitions(engine PUBLIC "-DENGINE_ENABLE_TEXT=1")
	set(TEXT_LIBRARY freetype)
	get_target_property(freetype_INCLUDE_DIRS freetype INTERFACE_INCLUDE_DIRECTORIES)
	target_include_directories(engine SYSTEM PUBLIC ${freetype_INCLUDE_DIRS})
else()
	target_compile_definitions(engine PUBLIC "-DENGINE_ENABLE_TEXT=0")
endif()

if(ENGINE_ENABLE_ANIMATION)
	include(external/ozz_animation.cmake)
	target_compile_definitions(engine PUBLIC "-DENGINE_ENABLE_ANIMATION=1")
else()
	target_compile_definitions(engine PUBLIC "-DENGINE_ENABLE_ANIMATION=0")
endif()

if (ENGINE_ENABLE_JSON)
	include(external/json.cmake)
	set(JSON_LIBRARY nlohmann_json::nlohmann_json)
	target_compile_definitions(engine PUBLIC "-DENGINE_ENABLE_JSON=1")
	get_target_property(json_INCLUDE_DIRS ${JSON_LIBRARY} INTERFACE_INCLUDE_DIRECTORIES)
	target_include_directories(engine SYSTEM PUBLIC ${json_INCLUDE_DIRS})
else()
	target_compile_definitions(engine PUBLIC "-DENGINE_ENABLE_JSON=0")
endif()

if (ENGINE_ENABLE_MULTITHREADED)
	include(external/fiber_tasking_lib.cmake)
	get_target_property(ftl_INCLUDE_DIRS ftl INTERFACE_INCLUDE_DIRECTORIES)
	target_include_directories(engine SYSTEM PUBLIC ${ftl_INCLUDE_DIRS})

	target_compile_definitions(engine PUBLIC "-DENGINE_ENABLE_MULTITHREADED=1")
	set(FTL_LIB ftl)
else()
	target_compile_definitions(engine PUBLIC "-DENGINE_ENABLE_MULTITHREADED=0")
endif()

target_compile_definitions(engine PUBLIC "-DENGINE_OS_APPLE=0")
target_compile_definitions(engine PUBLIC "-DENGINE_OS_WIN32=1")
target_compile_definitions(engine PUBLIC "-DENGINE_OS_LINUX=2")
if(APPLE)
	target_compile_definitions(engine PUBLIC "-DENGINE_OS=0")
elseif(WIN32)
	target_compile_definitions(engine PUBLIC "-DENGINE_OS=1")
else()
	target_compile_definitions(engine PUBLIC "-DENGINE_OS=2")
endif()

set(ENGINE_PRIV_DEPS stb_image ${GLFW3_LIBRARY})
set(ENGINE_PUB_DEPS glad glm OpenGL::GL
	# Add all components
	constants
	assimp::assimp
	${PRIV_VR_LIB}
	${FTL_LIB}
	# Add all of the optional libraries, will be empty string if disabled.
	${OPENVR_LIBRARY}
	${JSON_LIBRARY}
	${TEXT_LIBRARY}
	${OZZ_LIBRARIES}
	${OpenAl_DEPS})

if (ENGINE_DEBUG)
	target_compile_definitions(engine PUBLIC "-DENGINE_DEBUG=1")
else()
	target_compile_definitions(engine PUBLIC "-DENGINE_DEBUG=0")
endif()

if (ENGINE_CXX_OVERLOADS)
	target_compile_definitions(engine PUBLIC "-DENGINE_CXX_OVERLOADS=1")
else()
	target_compile_definitions(engine PUBLIC "-DENGINE_CXX_OVERLOADS=0")
endif()

if (ENGINE_MIN_GAME_OBJECT)
	target_compile_definitions(engine PUBLIC "-DENGINE_MIN_GAME_OBJECT=1")
else()
	target_compile_definitions(engine PUBLIC "-DENGINE_MIN_GAME_OBJECT=0")
endif()

target_include_directories(engine SYSTEM PUBLIC ${GLFW3_INCLUDE_DIR})
target_include_directories(engine PUBLIC include)
target_include_directories(engine SYSTEM PUBLIC ${OpenAl_INCLUDE_DIRECTORIES})
target_link_libraries(engine PUBLIC ${ENGINE_PUB_DEPS})
target_link_libraries(engine PRIVATE ${ENGINE_PRIV_DEPS})

# Optional linking to std::filesystem for gcc < 9.0
target_link_libraries(engine PRIVATE $<$<AND:$<CXX_COMPILER_ID:GNU>,$<VERSION_LESS:$<CXX_COMPILER_VERSION>,9.0>>:stdc++fs>)
target_link_libraries(engine PRIVATE $<$<AND:$<CXX_COMPILER_ID:Clang>,$<VERSION_LESS:$<CXX_COMPILER_VERSION>, 11.0>>:stdc++fs>)

target_compile_options(engine PRIVATE "${ENGINE_EXTRA_FLAGS}")

target_compile_definitions(engine PUBLIC "$<$<CONFIG:DEBUG>:DEBUG>")

if (ENGINE_ENABLE_TESTING)
	add_subdirectory(tests)
endif()
