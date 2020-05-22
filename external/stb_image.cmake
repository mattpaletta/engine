# Fetch STB Image
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/stb_image)
file(DOWNLOAD https://raw.githubusercontent.com/nothings/stb/master/stb_image.h ${CMAKE_CURRENT_BINARY_DIR}/stb_image/stb_image.h)
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/stb_image/stb_image.cpp
        "// This file was generated with CMake --- DO NOT EDIT
#define STB_IMAGE_IMPLEMENTATION\n
#include \"stb_image.h\"")
add_library(stb_image ${CMAKE_CURRENT_BINARY_DIR}/stb_image/stb_image.h ${CMAKE_CURRENT_BINARY_DIR}/stb_image/stb_image.cpp)
target_include_directories(stb_image PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

