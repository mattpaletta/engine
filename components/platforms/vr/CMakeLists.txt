if (ENGINE_ENABLE_VR)
	get_property(openvr_SOURCE_DIR GLOBAL PROPERTY openvr_SOURCE_DIR)
	
	file(GLOB SRC_FILES src/*.cpp)
	file(GLOB INC_FILES include/vr/*.hpp)

	add_library(vr ${SRC_FILES} ${INC_FILES})
	target_include_directories(vr PRIVATE include/vr)
	target_include_directories(vr PUBLIC include)
	target_include_directories(vr INTERFACE ${openvr_SOURCE_DIR}/headers)

	target_link_libraries(vr PUBLIC 
		constants
		${OPENVR_LIBRARY} Eigen3::Eigen)
	if (ENGINE_DEBUG_VR)
		target_compile_definitions(vr PUBLIC "-DENGINE_DEBUG_VR=1")
	else()
		target_compile_definitions(vr PUBLIC "-DENGINE_DEBUG_VR=0")
	endif()
endif()
