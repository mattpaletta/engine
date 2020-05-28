#include "engine/game.hpp"
#include "engine/engine.hpp"

Game::Game(const ScreenSize& _window_size, const std::string& window_name) : window_size(_window_size), name(window_name) {}
Game::Game(const int width, const int height, const std::string& window_name) : window_size(width, height), name(window_name) {}

void Game::ClearEngineDelegate() noexcept {}

void Game::SetEngineDelegate(Engine* enginePtr) {
	this->engine = enginePtr;
	this->window_size = this->engine->getScaledWindowSize();
}

void Game::Init() {}

void Game::Update([[maybe_unused]] const double& dt) noexcept {}

void Game::ProcessInput([[maybe_unused]] const double& dt) noexcept {}

void Game::pressed([[maybe_unused]] const int key) noexcept {}

void Game::released([[maybe_unused]] const int key) noexcept {}

void Game::Render() const noexcept {}
