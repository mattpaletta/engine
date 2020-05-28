#pragma once

#if ENGINE_OS == ENGINE_OS_APPLE
	#define GL_SILENCE_DEPRECATION
#endif

#include <GLFW/glfw3.h>
#include <iostream>
#include <string>

GLenum _glCheckError(const char* file, int line);
#define glCheckError() _glCheckError(__FILE__, __LINE__)
