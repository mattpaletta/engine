#pragma once

#include <string>
#include <memory>
#include <array>

#include "game_object.hpp"
#include "engine_fwd.hpp"
#include <constants/screen_size.hpp>

class Game {
protected:
	friend Engine;

	Engine* engine;
	GameObject player;

    void ClearEngineDelegate() noexcept;
	void SetEngineDelegate(Engine* engine);
public:
    ScreenSize window_size;
    std::string name;

    // Hold Game Input
    std::array<int, 1024> Keys;
    std::array<bool, 1024> KeysProcessed;

    // constructor/destructor
    Game(const ScreenSize& _window_size, const std::string& window_name);
    Game(const int width, const int height, const std::string& window_name);
    virtual ~Game() = default;

    // Delete move and copy operators.
    Game(const Game& g) = delete;
    Game& operator=(Game& g) = delete;
    //Game(Game&& g) = delete;
    //Game& operator=(Game&& g) = delete;

    // initialize game state (load all shaders/textures/levels)
    virtual void Init();

    // game loop
    virtual void ProcessInput(const double& dt) noexcept;
    virtual void Update(const double& dt) noexcept;
    virtual void Render() const noexcept;

    virtual void pressed(const int key) noexcept;
    virtual void released(const int key) noexcept;
};
