#pragma once

#include "components/components.hpp"
#include "common/common.hpp"
#include "ecs/ecs.hpp"

// create a button
Entity createButton(
	RenderSystem* renderer,
	vec2 position,
	vec2 size,
	vec3 color,
	std::string text_content,
	TextAlignment alignment,
	std::function<void(WorldSystem& world_system)> callback
);

Entity createText(RenderSystem* renderer, std::string content, vec2 pos, float scale, vec3 color, TextAlignment alignment);