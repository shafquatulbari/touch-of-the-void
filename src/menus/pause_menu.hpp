#pragma once

#include <vector>
#include <functional>

#include "common/common.hpp"
#include "ecs/ecs.hpp"
#include "render_system/render_system.hpp"
#include "world_system/world_system.hpp"

class PauseMenu {
public:
	static Entity title_entity;
	static std::vector<Entity> button_entities;
	static std::vector<Entity> text_entities;

	static bool has_state_changed;
	static bool is_hovering;
	static Entity hovered_entity;

	static void init(
		RenderSystem* renderer
	);

	static void close();
};