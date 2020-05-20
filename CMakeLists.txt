cmake_minimum_required(VERSION 3.11)

project(engine LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(ENGINE_ENABLE_AUDIO ON)
set(ENGINE_ENABLE_VR ON)
set(ENGINE_ENABLE_TEXT ON)
set(ENGINE_ENABLE_ANIMATION ON)
set(ENGINE_ENABLE_JSON ON)

# Include the addons
add_subdirectory(cmake)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

add_library(engine src/engine.cpp include/engine/engine.hpp)
include_directories(include)

find_package(OpenGL REQUIRED)
find_package(Threads REQUIRED)

include(external/glm.cmake)
include(external/stb_image.cmake)
include(external/glad.cmake)
find_package(GLFW3 REQUIRED)

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

set(ENGINE_DEPS glad glm OpenGL::GL stb_image ${GLFW3_LIBRARY} 
	# Add all of the optional libraries, will be empty string if disabled.
	${OPENVR_LIBRARY} 
	${JSON_LIBRARY} 
	${TEXT_LIBRARY} 
	${OZZ_LIBRARIES} 
	${OpenAl_DEPS})

target_include_directories(engine PUBLIC ${GLFW3_INCLUDE_DIR})
target_include_directories(engine PUBLIC include)
target_link_libraries(engine PUBLIC ${ENGINE_DEPS})

target_compile_definitions(engine PUBLIC "$<$<CONFIG:DEBUG>:DEBUG>")
