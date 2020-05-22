cmake_minimum_required(VERSION 3.11)

project(engine LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(ENGINE_ENABLE_AUDIO ON CACHE BOOL "Enable Audio Playback")
set(ENGINE_ENABLE_VR ON CACHE BOOL "Enable VR Support")
set(ENGINE_ENABLE_TEXT ON CACHE BOOL "Enable text rendering")
set(ENGINE_ENABLE_ANIMATION ON CACHE BOOL "Enable animation library")
set(ENGINE_ENABLE_JSON ON CACHE BOOL "Enable JSON decoding")
set(ENGINE_MIN_GAME_OBJECT OFF CACHE BOOL "Switch to minimum-game-object class")
set(ENGINE_ENABLE_MULTITHREADED OFF CACHE BOOL "Allow multithreaded engine, may impact performance")

# TODO: Debug each component?
set(ENGINE_DEBUG ON)
set(ENGINE_DEBUG_VR ON CACHE BOOL "Debug VR Components")

set(ENGINE_CXX_OVERLOADS ON)

# Include the addons
add_subdirectory(cmake)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

file(GLOB SRC_FILES src/*.cpp)
file(GLOB INC_FILES include/engine/*.hpp)
add_library(engine ${SRC_FILES} ${INC_FILES})
include_directories(include)

find_package(OpenGL REQUIRED)
find_package(Threads REQUIRED)

include(external/glm.cmake)
include(external/stb_image.cmake)
include(external/glad.cmake)
include(external/fiber_tasking_lib.cmake)
find_package(GLFW3 REQUIRED)

# VR Library depends on Eigen
include(external/eigen.cmake)

add_subdirectory(components/constants)

if (ENGINE_ENABLE_AUDIO)
	include(external/openal.cmake)
	target_compile_definitions(engine PUBLIC "-DENGINE_ENABLE_AUDIO=1")
endif()

if (ENGINE_ENABLE_VR)
	# Fetch OpenVR
	include(external/openvr.cmake)
	get_property(openvr_SOURCE_DIR GLOBAL PROPERTY openvr_SOURCE_DIR)
	target_include_directories(engine INTERFACE ${openvr_SOURCE_DIR}/headers)

	set(OPENVR_LIBRARY openvr_api64)
	target_compile_definitions(engine PUBLIC "-DENGINE_ENABLE_VR=1")
	
	# Add all platforms, will include VR
	add_subdirectory(components/platforms)
	
	set(PRIV_VR_LIB vr)
endif()

if (ENGINE_ENABLE_TEXT)
	include(external/freetype.cmake)
	target_compile_definitions(engine PUBLIC "-DENGINE_ENABLE_TEXT=1")
	set(TEXT_LIBRARY freetype)
endif()

if(ENGINE_ENABLE_ANIMATION)
	include(external/ozz_animation.cmake)
	target_compile_definitions(engine PUBLIC "-DENGINE_ENABLE_ANIMATION=1")
endif()

if (ENGINE_ENABLE_JSON)
	include(external/json.cmake)
	set(JSON_LIBRARY nlohmann_json::nlohmann_json) 
	target_compile_definitions(engine PUBLIC "-DENGINE_ENABLE_JSON=1")
endif()


set(ENGINE_DEPS glad glm OpenGL::GL stb_image ${GLFW3_LIBRARY} ftl
	# Add all components
	constants
	${PRIV_VR_LIB}
	# Add all of the optional libraries, will be empty string if disabled.
	${OPENVR_LIBRARY}
	${JSON_LIBRARY}
	${TEXT_LIBRARY}
	${OZZ_LIBRARIES}
	${OpenAl_DEPS})

if (ENGINE_DEBUG)
	target_compile_definitions(engine PUBLIC "-DENGINE_DEBUG=1")
endif()

if (ENGINE_CXX_OVERLOADS)
	target_compile_definitions(engine PUBLIC "-DENGINE_CXX_OVERLOADS=1")
endif()

if (ENGINE_MIN_GAME_OBJECT)
	target_compile_definitions(engine PUBLIC "-DENGINE_MIN_GAME_OBJECT=1")
else()
	target_compile_definitions(engine PUBLIC "-DENGINE_MIN_GAME_OBJECT=0")
endif()

if (ENGINE_ENABLE_MULTITHREADED)
	# FTL is included every time.
	target_compile_definitions(engine PUBLIC "-DENGINE_ENABLE_MULTITHREADED=1")
else()
	target_compile_definitions(engine PUBLIC "-DENGINE_ENABLE_MULTITHREADED=0")
endif()

target_include_directories(engine PUBLIC ${GLFW3_INCLUDE_DIR})
target_include_directories(engine PUBLIC include)
target_link_libraries(engine PUBLIC ${ENGINE_DEPS})

target_compile_definitions(engine PUBLIC "$<$<CONFIG:DEBUG>:DEBUG>")
