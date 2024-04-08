#include "shop_menu.hpp"

// Defining static variables
Entity ShopMenu::current_hovered;
Entity ShopMenu::previous_hovered;
bool ShopMenu::is_hovering = false;
bool ShopMenu::has_state_changed = false;

ITEM_TYPE ShopMenu::current_page = ITEM_TYPE::WEAPON;

Entity ShopMenu::balance_text_e;
WeaponType ShopMenu::weapon_on_sale;

std::vector<Entity> ShopMenu::button_entities;
std::vector<Entity> ShopMenu::text_entities;

std::vector<Entity> ShopMenu::list_button_entities;
std::vector<Entity> ShopMenu::list_text_entities;

void ShopMenu::init(RenderSystem* renderer, WeaponType weapon_on_sale) {
	// Initialize data
	ShopMenu::weapon_on_sale = weapon_on_sale;
	std::vector<WeaponType> unlocked_weapons;

	Player& player = registry.players.components.back();
	for (int i = 0; i < (int)WeaponType::TOTAL_WEAPON_TYPES; i++) {
		if (player.total_ammo_count[(WeaponType)i] != INT_MIN && (WeaponType)i != WeaponType::GATLING_GUN) {
			unlocked_weapons.push_back((WeaponType)i);
		}
	}
	
	// Create a balance text
	int text_scale = 1.5f;
	balance_text_e = createText(renderer,
		"Funds: " + std::to_string(player.funds), {window_width_px - 100.f, 100.f}, text_scale, COLOR_GREEN, TextAlignment::RIGHT
	);

	registry.renderRequests.emplace(balance_text_e).used_render_layer = RENDER_LAYER::GAME_MENU;

	// Create a close window button
	Entity close_btn_e = createButton(
		renderer, {100.f, 100.f}, {100.f, 100.f}
	);

	Motion& close_motion = registry.motions.get(close_btn_e);
	Entity close_text_e = createText(
		renderer, "X", close_motion.position, 1.5f, COLOR_GREEN, TextAlignment::CENTER
	);

	registry.renderRequests.emplace(close_text_e).used_render_layer = RENDER_LAYER::GAME_MENU;

	text_entities.push_back(close_text_e);
	button_entities.push_back(close_btn_e);

	// Create the tab buttons
	float tab_btn_left_offset = close_motion.position.x + close_motion.scale.x / 2 + 175.f;
	float tab_btn_gap = 150.f;

	vec2 prev_pos = {tab_btn_left_offset, 100.f};
	std::string tab_title[] = {"Weapon", "Ammo", "Misc."};
	for (int i = 0; i < (int)ITEM_TYPE::TYPE_COUNT; i++) {
		Entity tab_btn_e = createButton(
			renderer, prev_pos, { 125.f, 100.f }
		);

		Motion& tab_btn_motion = registry.motions.get(tab_btn_e);
		Entity tab_btn_text_e = createText(
			renderer, tab_title[i], prev_pos, 1.5f, COLOR_GREEN, TextAlignment::CENTER
		);

		registry.renderRequests.emplace(tab_btn_text_e).used_render_layer = RENDER_LAYER::GAME_MENU;
		Button& tab_btn = registry.buttons.get(tab_btn_e);
		tab_btn.on_click = []() {
			
		};
		
		button_entities.push_back(tab_btn_e);
		text_entities.push_back(tab_btn_text_e);

		prev_pos.x += 100.f + tab_btn_gap;
	}
}


bool ShopMenu::on_mouse_move(vec2 mouse_position) {
	has_state_changed = false;

	previous_hovered = current_hovered;
	for (int i = 0; i < button_entities.size(); i++) {
		Entity& e = button_entities[i];

		Motion& motion = registry.motions.get(e);
		Button& button = registry.buttons.get(e);

		if (
			mouse_position.x <= motion.position.x + motion.scale.x / 2 &&
			mouse_position.x >= motion.position.x - motion.scale.x / 2 &&
			mouse_position.y <= motion.position.y + motion.scale.y / 2 &&
			mouse_position.y >= motion.position.y - motion.scale.y / 2
		) {
			ShopMenu::has_state_changed = true;
			ShopMenu::is_hovering = true;
			ShopMenu::current_hovered = e;

			Text& text = registry.texts.get(text_entities[i]);
			text.color = COLOR_RED;

			return true;
		}
	}

	if (!has_state_changed || previous_hovered != current_hovered) {
		is_hovering = false;

		if (!registry.buttons.has(previous_hovered)) {
			return false;
		}

		for (int i = 0; i < button_entities.size(); i++) {
			if (previous_hovered == button_entities[i]) {
				Text& text = registry.texts.get(text_entities[i]);
				text.color = COLOR_GREEN;
				break;
			}
		}
	}

	return false;
}

void ShopMenu::on_mouse_click(int button, int action, int mods) {
	if (is_hovering) {
		Button& button = registry.buttons.get(current_hovered);
		button.on_click();
	}
}

void ShopMenu::close() {
	// Reset states
	current_hovered;
	previous_hovered;
	is_hovering = false;
	has_state_changed = false;

	current_page = ITEM_TYPE::WEAPON;

	// Remove all related components
	registry.remove_all_components_of(balance_text_e);

	for (Entity e : text_entities) {
		registry.remove_all_components_of(e);
	}

	for (Entity e : button_entities) {
		registry.remove_all_components_of(e);
	}

	button_entities.clear();
	text_entities.clear();

	list_text_entities.clear();
	list_button_entities.clear();
}