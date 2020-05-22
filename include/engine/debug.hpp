#pragma once

#include <GLFW/glfw3.h>
#include <iostream>
#include <string>

GLenum _glCheckError(const char* file, int line);
#define glCheckError() _glCheckError(__FILE__, __LINE__)