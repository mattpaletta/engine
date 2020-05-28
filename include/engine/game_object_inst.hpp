#pragma once
#include <vector>

#include <constants/position.hpp>
#include "engine/game_object.hpp"

template<typename T>
class GameObjectInst {
public:
	T game_object;
	GameObjectInst() = default;
	~GameObjectInst() = default;
};