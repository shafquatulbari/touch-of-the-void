#pragma once

#include "render_system/render_system.hpp"
#include "components/components.hpp"
#include "common/common.hpp"
#include "ecs/ecs.hpp"
#include "ecs_registry/ecs_registry.hpp"


// create a button
Entity createButton(
	RenderSystem* renderer,
	vec2 position,
	vec2 size,
	vec3 color,
	std::string text_content,
	float font_size,
	TextAlignment alignment,
	std::function<void(void)> callback
);

Entity createText(RenderSystem* renderer, std::string content, vec2 pos, float scale, vec3 color, TextAlignment alignment);