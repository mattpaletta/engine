#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#if ENGINE_ENABLE_JSON
#include <nlohmann/json.hpp>
#endif

#if ENGINE_ENABLE_VR
#include <vr/vr.hpp>
#endif

#include "game.hpp"
#include "sprite.hpp"
#include "audio.hpp"
#include "text_renderer.hpp"
#include "resource.hpp"
#include "scheduler.hpp"
#include "3d_renderer.hpp"
#include "light_manager.hpp"

#include <constants/size.hpp>
#include <constants/screen_size.hpp>

class Engine {
private:
	ScreenSize SCREEN_SIZE;
	std::shared_ptr<Game> game = nullptr;

	std::unique_ptr<Renderer3D> renderer3d = nullptr;
    LightManager lightManager;
    std::unique_ptr<SpriteRenderer> spriteRenderer = nullptr;
	AudioEngine audioEngine;
	std::unique_ptr<TextRenderer> textRenderer = nullptr;
	ResourceManager resourceManager;
	Scheduler scheduler;

#if ENGINE_ENABLE_VR
	VRApplication vr;
#endif
	unsigned short leftStrength;
	unsigned short rightStrength;

	double deltaTime = 0.0;
	double lastFrame = 0.0;

	// OpenGL Window
	GLFWwindow* app_window;

	void init_opengl();
	void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);

	// Utility Functions
	int getScaledWidth() const;
	int getScaledHeight() const;
	double getScaleRatio() const;

	// Debug
	Colour clearColour = Colour::black;
public:
	Engine(std::shared_ptr<Game> _g);
	Engine(const ScreenSize& size, std::shared_ptr<Game> _g);
	~Engine();

	// Delete Copy Operators
	Engine(const Engine&) = delete;
	Engine& operator=(const Engine&) = delete;

	// Utility functions
	Size scaleObj(const Size& desired_size) const;
	float scaleConst(const float& desired_size) const;
	ScreenSize getScaledWindowSize() const;

	// Window customization
	void resizeable(bool value);
	void enableBlending() const;

	// Runs the Render Loop
	void run();

	// Debug
	void setClearColour(const Colour& colour);


	// 3D
	Renderer3D* get3DRenderer();
    // Lighting (3D)
    LightManager* getLightManager();

	// Sprites
	void setCustomSpriteRendering(const std::string& resourceName);
	void enableSpriteRendering(const bool is_enabled = true);
	SpriteRenderer* getSpriteRenderer();

	// Audio
	AudioEngine* getAudioEngine();

	// Text
	TextRenderer* getTextRenderer();

	// Resources
	ResourceManager* getResourceManager();

	// Resources
	Scheduler* getScheduler();

	// VR
	void Update_VR_vibration(const unsigned short leftStrength, const unsigned short rightStrength);
	bool Is_VR_vibrating();
};
