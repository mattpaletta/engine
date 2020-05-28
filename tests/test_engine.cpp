#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

#include <engine/engine.hpp>
#include <constants/screen_size.hpp>
#include <engine/game.hpp>

TEST_CASE("startup", "[engine]") {
	const ScreenSize size { 800, 600 };
	std::shared_ptr<Game> g = std::make_shared<Game>(size, "test_engine");
	Engine e{g};
}
