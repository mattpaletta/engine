#include "engine/game.hpp"
#include "engine/engine.hpp"

Game::Game(const ScreenSize& _window_size, const std::string& window_name) : window_size(_window_size), name(window_name) {}
Game::Game(const int width, const int height, const std::string& window_name) : window_size(width, height), name(window_name) {}

void Game::ClearEngineDelegate() noexcept {}

void Game::SetEngineDelegate(Engine* engine) {
	this->engine = engine;
	this->window_size = engine->getScaledWindowSize();
}

void Game::Init() {}

void Game::Update(const double& dt) noexcept {}

void Game::ProcessInput(const double& dt) noexcept {}

void Game::pressed(const int key) noexcept {}

void Game::released(const int key) noexcept {}

void Game::Render() const noexcept {}