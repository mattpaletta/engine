#pragma once

class Renderer {
public:
	Renderer() = default;
	~Renderer() = default;

	// Not copyable, only moveable
	Renderer(const Renderer&) = delete;
	Renderer& operator=(const Renderer&) = delete;
	Renderer(Renderer&&) = default;
};