#include "engine/engine.hpp"
#include "engine/debug.hpp"

#include <memory>
#include <glad/glad.h>

// Helper function for opengl
void framebuffer_size_callback([[maybe_unused]] GLFWwindow* window, int width, int height) {
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
#if ENGINE_DEBUG
    std::cout << "Called framebuffer_size_callback" << std::endl;
#endif
    glViewport(0, 0, width, height);
    glCheckError();
}

Engine::Engine(std::shared_ptr<Game> _g) : SCREEN_SIZE(_g->window_size), game(_g), audioEngine() {
    this->init_opengl();
    this->game->SetEngineDelegate(this);

    // Configure the game
    this->game->Init();
}

Engine::Engine(const ScreenSize& size, std::shared_ptr<Game> _g) : SCREEN_SIZE(size), game(_g), spriteRenderer() {
    this->init_opengl();
    this->game->SetEngineDelegate(this);

    // Configure the game
    this->game->Init();
}

Engine::~Engine() {
    this->game->ClearEngineDelegate();
}

void Engine::setCustomSpriteRendering(const std::string& resourceName) {
    this->spriteRenderer = SpriteRenderer::UniqueFromCustomShader(this->resourceManager.GetShader(resourceName, __FILE__, __LINE__));
}

void Engine::key_callback(GLFWwindow* window, int key, [[maybe_unused]] int scancode, int action, [[maybe_unused]] int mode) {
    // When a user presses the escape key, we set the WindowShouldClose property to true, closing the application
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }

    if (key >= 0 && key < 1024) {
        if (action == GLFW_PRESS) {
            // Update 'pressed' after the bool array
            this->game->Keys[static_cast<std::size_t>(key)] = true;
            this->game->pressed(key);

        } else if (action == GLFW_RELEASE) {
            this->game->Keys[static_cast<std::size_t>(key)] = false;
            this->game->KeysProcessed[static_cast<std::size_t>(key)] = false;
            this->game->released(key);
        }
    }
}

void Engine::init_opengl() {
    // Initialize OpenGL
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

#if ENGINE_DEBUG
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
#endif

    // Default to false
    this->resizeable(false);
#ifdef __APPLE__
    glfwWindowHint(GLFW_SCALE_TO_MONITOR, false);
#endif

    this->app_window = glfwCreateWindow(this->SCREEN_SIZE.WIDTH, this->SCREEN_SIZE.HEIGHT, this->game->name.c_str(), nullptr, nullptr);
    if (this->app_window == nullptr) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        throw std::runtime_error("Failed to create GLFW window");
    }
    glfwMakeContextCurrent(this->app_window);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        throw std::runtime_error("Failed to initialize GLAD");
    }

    // Hack to get the engine to register
    glfwSetWindowUserPointer(this->app_window, this);
    auto key_callback_lambda = [](GLFWwindow* window, int key, int scancode, int action, int mode) {
        Engine* e = static_cast<Engine*>(glfwGetWindowUserPointer(window));
        e->key_callback(window, key, scancode, action, mode);
    };

    glfwSetKeyCallback(this->app_window, key_callback_lambda);
    glfwSetFramebufferSizeCallback(this->app_window, framebuffer_size_callback);

    // OpenGL configuration
    // --------------------
    ScreenSize scaled_size;
    glfwGetFramebufferSize(this->app_window, &scaled_size.WIDTH, &scaled_size.HEIGHT);
    glViewport(0, 0, scaled_size.WIDTH, scaled_size.HEIGHT);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);

#if ENGINE_ENABLE_TEXT
    // Initalize TextRenderer after OpenGL has been initializd
    this->textRenderer = std::make_unique<TextRenderer>();
    this->textRenderer->Init(this, scaled_size);
#endif

#if ENGINE_ENABLE_VR
    this->vr.Init();
#endif

    this->renderer3d = std::make_unique<Renderer3D>();
}

void Engine::setClearColour(const Colour& colour) {
    this->clearColour = colour;
}

double Engine::getScaleRatio() const {
    ScreenSize scaled_size;
    glfwGetFramebufferSize(this->app_window, &scaled_size.WIDTH, &scaled_size.HEIGHT);
    return scaled_size.WIDTH / this->SCREEN_SIZE.WIDTH;
}

int Engine::getScaledWidth() const {
    return static_cast<int>(std::nearbyint(this->SCREEN_SIZE.WIDTH * this->getScaleRatio()));
}

int Engine::getScaledHeight() const {
    return static_cast<int>(std::nearbyint(this->SCREEN_SIZE.HEIGHT * this->getScaleRatio()));
}

float Engine::scaleConst(const float& desired_size) const {
    return desired_size * static_cast<float>(this->getScaleRatio());
}

Size Engine::scaleObj(const Size& desired_size) const {
    const float SCALE_CONSTANT = static_cast<float>(std::round(this->getScaleRatio()));
    return { desired_size.Width * SCALE_CONSTANT, desired_size.Height * SCALE_CONSTANT };
}

ScreenSize Engine::getScaledWindowSize() const {
    return { this->getScaledWidth(), this->getScaledHeight() };
}

void Engine::resizeable(bool value) {
    glfwWindowHint(GLFW_RESIZABLE, value);
}

void Engine::run() {
    this->deltaTime = 0;
    this->lastFrame = 0;

    auto stopCondition = [this]() {
#if ENGINE_ENABLE_VR
        return !glfwWindowShouldClose(this->app_window) || this->vr.ShouldShutdown();
#else
        return !glfwWindowShouldClose(this->app_window);
#endif
    };

    // Do the game loop
    while (stopCondition()) {
        // calculate delta time
        // --------------------
        const double currentFrame = glfwGetTime();
        this->deltaTime = currentFrame - lastFrame;
        this->lastFrame = currentFrame;
        glfwPollEvents();

        // manage user input
        // -----------------
#if ENGINE_ENABLE_VR
        this->vr.GetInput();
#endif
        this->game->ProcessInput(deltaTime);
#if ENGINE_DEBUG
        glCheckError();
#endif

        // update game state
        // -----------------
        this->game->Update(deltaTime);
#if ENGINE_DEBUG
        glCheckError();
#endif

#if ENGINE_ENABLE_VR
        this->vr.RunVibration(this->leftStrength, this->rightStrength);
#endif
        // render
        // ------
        glClearColor(clearColour.R, clearColour.G, clearColour.B, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        this->game->Render();
#if ENGINE_DEBUG
        glCheckError();
#endif
        // Run Audio Tick
        this->audioEngine.Update();

        glfwSwapBuffers(this->app_window);

#if ENGINE_ENABLE_VR
        // Fetch new HMD position
        this->vr.GetTrackingPose();
#endif
}

    // delete all resources as loaded using the resource manager
    // ---------------------------------------------------------
    this->resourceManager.Clear();

    glfwTerminate();
}

Renderer3D* Engine::get3DRenderer() {
    if (this->renderer3d) {
        return this->renderer3d.get();
    } else {
        return nullptr;
    }
}

void Engine::enableSpriteRendering(const bool is_enabled) {
    if (is_enabled) {
        this->spriteRenderer = std::make_unique<SpriteRenderer>();
    } else {
        this->spriteRenderer.release();
        this->spriteRenderer = nullptr;
    }
}

SpriteRenderer* Engine::getSpriteRenderer() {
    if (this->spriteRenderer) {
        return this->spriteRenderer.get();
    } else {
        return nullptr;
    }
}

// MARK: Audio
AudioEngine* Engine::getAudioEngine() {
    return &this->audioEngine;
}

// MARK: Text
TextRenderer* Engine::getTextRenderer() {
    return this->textRenderer.get();
}

ResourceManager* Engine::getResourceManager() {
    return &this->resourceManager;
}

Scheduler* Engine::getScheduler() {
    return &this->scheduler;
}

void Engine::Update_VR_vibration([[maybe_unused]] const unsigned short newLeftStrength, [[maybe_unused]] const unsigned short newRightStrength) {
#if ENGINE_ENABLE_VR
    this->leftStrength = newLeftStrength;
    this->rightStrength = newRightStrength;
#endif
}

bool Engine::Is_VR_vibrating() {
    return (this->leftStrength + this->rightStrength) > 0;
}
