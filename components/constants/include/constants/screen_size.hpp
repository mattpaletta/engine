#pragma once

#include <iostream>

struct ScreenSize final {
	int WIDTH;
	int HEIGHT;

	ScreenSize() : WIDTH(0), HEIGHT(0) {}
	ScreenSize(const int width, const int height) : WIDTH(width), HEIGHT(height) {}
	~ScreenSize() = default;
};