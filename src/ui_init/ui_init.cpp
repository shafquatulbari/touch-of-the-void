#include <functional>

#include "world_system//world_system.hpp"
#include "world_init/world_init.hpp"

Entity createTextButton(
	RenderSystem* renderer,
	vec2 position,
	vec2 size,
	vec3 color,
	std::string text_content,
	float font_size,
	TextAlignment alignment,
	std::function<void(void)> on_click
) {
	auto entity = Entity();

	// Create a button component
	Button& button = registry.buttons.emplace(entity);

	// Set the position of the button
	Motion& motion = registry.motions.emplace(entity);
	motion.position = position;
	motion.scale = size;

	// create the button's text
	//vec2 text_pos = { size.x + 0.2 * size.y, position.y - 0.2 * size.y }; // Position text with a padding 20% of the button's height
	vec2 text_pos = { position.x, position.y };
	float text_scale = font_size;
	
	Entity text_e = createText(renderer, text_content, text_pos, text_scale, color, alignment);
	RenderRequest& renderReq = registry.renderRequests.emplace(text_e);
	renderReq.used_render_layer = RENDER_LAYER::GAME_MENU;
	
	// Bind button to callback and Text entity
	button.text_entity = text_e;
	button.on_click = on_click;

	registry.noCollisionChecks.emplace(entity);

	return entity;
}

Entity createTextureButton(
	RenderSystem* renderer,
	vec2 position,
	vec2 size,
	vec3 color,
	TEXTURE_ASSET_ID texture,
	std::function<void(void)> on_click
) {
	auto entity = Entity();

	Button& button = registry.buttons.emplace(entity);
	RenderRequest& renderRequest = registry.renderRequests.insert(
		entity, 
		{ texture,
		EFFECT_ASSET_ID::TEXTURED,
		GEOMETRY_BUFFER_ID::SPRITE,
		RENDER_LAYER::GAME_MENU });

	button.on_click = on_click;

	return entity;
}

Entity createButton(
	RenderSystem* renderer,
	vec2 position,
	vec2 size
) {
	auto entity = Entity();

	Button& button = registry.buttons.emplace(entity);

	Motion& motion = registry.motions.emplace(entity);
	motion.position = position;
	motion.scale = size;

	return entity;
}