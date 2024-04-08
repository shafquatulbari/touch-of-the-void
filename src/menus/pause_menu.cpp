#include "pause_menu.hpp"

#include "world_init/world_init.hpp"
#include "ui_init/ui_init.hpp"


// Initialize static variables
Entity PauseMenu::title_entity;
std::vector<Entity> PauseMenu::button_entities;
std::vector<Entity> PauseMenu::text_entities;

bool	PauseMenu::is_hovering = false;
bool	PauseMenu::has_state_changed = false;
Entity	PauseMenu::hovered_entity;

void PauseMenu::init(
	RenderSystem* renderer
) {
	// Create the title
	Entity title_e = createText(renderer, "Pause",
		{ window_width_px / 2, 150 }, 2.f, {1.f, 0.f, 0.f},
		TextAlignment::CENTER
	);

	registry.renderRequests.emplace(title_e).used_render_layer = RENDER_LAYER::GAME_MENU;


	int offset_multiplier = 1;
	// Create buttons
	// Resume button
	Entity resume_e = createText(
		renderer, "Resume",
		{ window_width_px / 2, offset_multiplier * window_height_px / 4 },
		1.25f,
		{1.f, 0.f, 0.f},
		TextAlignment::CENTER
	);

	registry.renderRequests.emplace(resume_e).used_render_layer = RENDER_LAYER::GAME_MENU;

	Motion& resume_motion = registry.motions.get(resume_e);
	Entity resume_btn_e = createButton(
		renderer, {resume_motion.position.x, window_height_px - resume_motion.position.y},
		{ 675, 112 }
	);

	offset_multiplier++;
	
	// Exit game button
	Entity exit_e = createText(
		renderer, "Exit to main menu",
		{ window_width_px / 2, offset_multiplier * window_height_px / 4 },
		1.25f,
		{ 1.f, 0.f, 0.f },
		TextAlignment::CENTER
	);

	registry.renderRequests.emplace(exit_e).used_render_layer = RENDER_LAYER::GAME_MENU;

	Motion& exit_motion = registry.motions.get(exit_e);
	Entity exit_btn_e = createButton(
		renderer, {exit_motion.position.x, window_height_px - exit_motion.position.y },
		{675, 112}
	);

	offset_multiplier++;

	// Close window button
	Entity close_e = createText(
		renderer, "Exit to Desktop",
		{ window_width_px / 2, offset_multiplier * window_height_px / 4 },
		1.25f,
		{ 1.f, 0.f, 0.f },
		TextAlignment::CENTER
	);

	registry.renderRequests.emplace(close_e).used_render_layer = RENDER_LAYER::GAME_MENU;

	Motion& close_motion = registry.motions.get(close_e);
	Entity close_btn_e = createButton(
		renderer, {close_motion.position.x, window_height_px - close_motion.position.y},
		{675, 112}
	);

	title_entity = title_e;
	text_entities = { resume_e, exit_e, close_e };
	button_entities = { resume_btn_e, exit_btn_e, close_btn_e };
}

void PauseMenu::close() {
	registry.remove_all_components_of(title_entity);

	for (Entity e : text_entities) {
		registry.remove_all_components_of(e);
	}

	for (Entity e : button_entities) {
		registry.remove_all_components_of(e);
	}

	text_entities.clear();
	button_entities.clear();
}