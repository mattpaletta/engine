file(GLOB SRC_FILES src/*.cpp)
file(GLOB INC_FILES include/constants/*.hpp)

add_library(constants ${SRC_FILES} ${INC_FILES})
target_include_directories(constants PRIVATE include/constants)
target_include_directories(constants PUBLIC include)
target_link_libraries(constants PUBLIC glad glm OpenGL::GL)