#pragma once

#include "ecs/ecs.hpp"
#include "common/common.hpp"
#include "components/components.hpp"

class ShopMenu {
public:
	static std::vector<Entity> text_entities;
	static std::vector<Entity> button_entities;

	static void init(Player& player);
	static void close();
};