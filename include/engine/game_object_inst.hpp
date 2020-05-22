#pragma once
#include <vector>

#include <constants/position.hpp>
#include "engine/game_object.hpp"

template<typename T, typename Pos = Position3d>
class GameObjectInst {
public:
	T game_object;
	std::vector<Pos> positions;
	GameObjectInst() = default;
	~GameObjectInst() = default;
};