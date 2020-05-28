file(GLOB SRC_FILES src/*.cpp)
file(GLOB INC_FILES include/constants/*.hpp)


# Try and find filesystem (solved in code directly for now)
#check_cxx_source_compiles(
#	"#include <filesystem>
#	int main() {
#		std::filesystem::path p;
#		return 0;
#	}"
#	did_find_filesystem
#	CMAKE_REQUIRED_INCLUDES filesystem)
#if (NOT did_find_filesystem)
#	check_cxx_source_compiles(
#		"#include <experimental/filesystem>
#		int main() {
#			std::experimental::filesystem::path p;
#			return 0;
#		}"
#		did_find_filesystem_experimental
#		CMAKE_REQUIRED_INCLUES experimental/filesystem)
#else()
#	set(did_find_filesystem_experimental OFF)
#endif()

add_library(constants ${SRC_FILES} ${INC_FILES})
target_include_directories(constants PRIVATE include/constants)
target_include_directories(constants PUBLIC include)

if (ENGINE_DEBUG)
	target_compile_definitions(constants PUBLIC "-DENGINE_DEBUG=1")
else()
	target_compile_definitions(constants PUBLIC "-DENGINE_DEBUG=0")
endif()

if (ENGINE_CXX_OVERLOADS)
	target_compile_definitions(constants PUBLIC "-DENGINE_CXX_OVERLOADS=1")
else()
	target_compile_definitions(constants PUBLIC "-DENGINE_CXX_OVERLOADS=0")
endif()

get_target_property(glm_INCLUDE_DIRS glm INTERFACE_INCLUDE_DIRECTORIES)
target_include_directories(constants SYSTEM PUBLIC ${glm_INCLUDE_DIRS})

target_link_libraries(constants PUBLIC glad glm OpenGL::GL)
target_compile_options(constants PRIVATE ${EXTRA_FLAGS})
