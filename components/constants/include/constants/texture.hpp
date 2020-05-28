#pragma once

#include <constants/screen_size.hpp>

// Texture2D is able to store and configure a texture in OpenGL.
// It also hosts utility functions for easy management.
class Texture2D {
public:
    std::string desc; // Optional description of texture

    // holds the ID of the texture object, used for all texture operations to reference to this particlar texture
    unsigned int ID;

    // texture image dimensions
    ScreenSize size; // width and height of loaded image in pixels

    // texture Format
    int Internal_Format; // format of texture object
    unsigned int Image_Format; // format of loaded image

    // texture configuration
    int Wrap_S; // wrapping mode on S axis
    int Wrap_T; // wrapping mode on T axis
    int Wrap_R; // wrapping mode on R axis
    int Filter_Min; // filtering mode if texture pixels < screen pixels
    int Filter_Max; // filtering mode if texture pixels > screen pixels

    // constructor (sets default texture modes)
    Texture2D();

    // generates texture from image data
    void Generate(const ScreenSize& screen_size, unsigned char* data);

    // Generate Cubemap (requires special parameters)
    void GenerateCubeMapInit(const ScreenSize& screen_size);
    void GenerateCubeMapFace(const int i, unsigned char* data);
    void GenerateCubeMapCleanup();

    // binds the texture as the current active GL_TEXTURE_2D texture object
    void Bind() const;
    void BindCubeMap() const;
};
