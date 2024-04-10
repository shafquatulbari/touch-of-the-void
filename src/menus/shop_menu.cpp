#include <iostream>

#include "shop_menu.hpp"
#include "shop_system/shop_constants.hpp"
#include "weapon_system/weapon_system.hpp"

// Defining static variables
RenderSystem* ShopMenu::renderer;

ITEM_TYPE ShopMenu::current_page = ITEM_TYPE::WEAPON;

Entity ShopMenu::balance_text_e;
WeaponType ShopMenu::weapon_on_sale;
std::vector<WeaponType> ShopMenu::unlocked_weapons;

std::vector<Entity> ShopMenu::button_entities;
std::vector<Entity> ShopMenu::text_entities;

std::vector<Entity> ShopMenu::page_entities;
std::vector<Entity> ShopMenu::list_entities;

void ShopMenu::init(
	RenderSystem* renderer, 
	WeaponType weapon_on_sale,
	std::function<void()> close_shop_callback
) {
	// Initialize data
	ShopMenu::renderer = renderer;
	ShopMenu::weapon_on_sale = weapon_on_sale;
	ShopMenu::current_page = ITEM_TYPE::WEAPON;

	Player& player = registry.players.components.back();
	for (int i = 0; i < (int)WeaponType::TOTAL_WEAPON_TYPES; i++) {
		if (!is_weapon_locked((WeaponType)i)) {
			unlocked_weapons.push_back((WeaponType)i);
		}
	}
	
	// Create a balance text
	int text_scale = 1.5f;
	balance_text_e = createText(renderer,
		"Gold Balance: " + std::to_string(player.gold_balance), {window_width_px - 100.f, 100.f}, text_scale, COLOR_GREEN, TextAlignment::RIGHT
	);

	registry.renderRequests.emplace(balance_text_e).used_render_layer = RENDER_LAYER::GAME_MENU;

	// Create a close window button
	Entity close_text_e = createText(
		renderer, "X", {100.f, 100.f}, 1.5f, COLOR_RED, TextAlignment::CENTER
	);
	
	Text& close_text = registry.texts.get(close_text_e);
	Motion& close_text_m = registry.motions.get(close_text_e);
	
	Entity close_btn_e = createButton(
		renderer, { close_text_m.position.x, window_height_px - close_text_m.position.y - close_text.rect_size.y / 2 }, close_text.rect_size
	);

	bind_button_hover(close_btn_e, close_text_e, COLOR_RED, COLOR_GREEN);

	registry.renderRequests.emplace(close_text_e).used_render_layer = RENDER_LAYER::GAME_MENU;
	registry.buttons.get(close_btn_e).on_click = close_shop_callback;

	button_entities.push_back(close_btn_e);
	text_entities.push_back(close_text_e);

	std::cout << "Close text rect: " << close_text.rect_size.x << " " << close_text.rect_size.y << '\n';
	std::cout << "Close position: " << close_text_m.position.x << " " << close_text_m.position.y << '\n';

	// Create the tab buttons
	Motion& close_btn_m = registry.motions.get(close_btn_e);
	vec2 prev_pos = { close_btn_m.position.x + close_btn_m.scale.x + 75.f, window_height_px - close_text_m.position.y };
	
	std::string tab_title[] = {"Weapon", "Ammo", "Misc."};
	std::function<void()> tab_callbacks[] = {render_weapons_page, render_ammo_page, render_misc_page};
	
	for (int i = 0; i < (int)ITEM_TYPE::TYPE_COUNT; i++) {
		Entity tab_btn_text_e = createText(
			renderer, tab_title[i], prev_pos, 1.5f, COLOR_RED, TextAlignment::LEFT
		);

		Text& text = registry.texts.get(tab_btn_text_e);
		Entity tab_btn_e = createButton(
			renderer, prev_pos + vec2({ text.rect_size.x / 2, -text.rect_size.y / 2 }), text.rect_size
		);

		registry.renderRequests.emplace(tab_btn_text_e).used_render_layer = RENDER_LAYER::GAME_MENU;
		Button& tab_btn = registry.buttons.get(tab_btn_e);
		tab_btn.on_click = tab_callbacks[i];
		
		bind_button_hover(tab_btn_e, tab_btn_text_e, COLOR_RED, COLOR_GREEN);

		button_entities.push_back(tab_btn_e);
		text_entities.push_back(tab_btn_text_e);

		prev_pos.x += (text.rect_size.x + 100.f);
	}

	render_weapons_page();
}

void ShopMenu::clear_page_entities() {
	for (Entity e : page_entities) {
		registry.remove_all_components_of(e);
	}

	page_entities.clear();
}

Entity ShopMenu::createStatText(std::string content, std::string value, vec2 position) {
	Entity e = createText(
		renderer, content + value, position, 0.75f, COLOR_GREEN, TextAlignment::LEFT
	);
	
	registry.renderRequests.emplace(e).used_render_layer = RENDER_LAYER::GAME_MENU;
	
	list_entities.push_back(e);
	page_entities.push_back(e);
	
	return e;
}

void ShopMenu::render_weapons_page() {
	clear_page_entities();

	// Page when all weapons are unlocked
	if (weapon_on_sale == WeaponType::TOTAL_WEAPON_TYPES) {
		Entity display_text_e = createText(
			renderer, "You have unlocked all weapons", 
			{ window_width_px / 2, window_height_px / 2 }, 2.5f, COLOR_GREEN, TextAlignment::CENTER
		);
		
		registry.renderRequests.emplace(display_text_e).used_render_layer = RENDER_LAYER::GAME_MENU;
		page_entities.push_back(display_text_e);
	}

	// Get the information of the weapon being sold
	WeaponShopInfo w_info = weapon_shop_infos[ShopMenu::weapon_on_sale];
	WeaponStats w_stat = weapon_stats[ShopMenu::weapon_on_sale];
	
	std::cout << (int)w_info.texture << " " << w_info.weapon_name << std::endl;
	std::cout << w_stat.damage << "-" << w_stat.magazine_size << std::endl;

	TEXTURE_ASSET_ID x = weapon_shop_infos[WeaponType::GATLING_GUN].texture;

	// Create the weapon icon
	Entity weapon_icon_e = createWeaponEquippedIcon(renderer, { 354.f, 420.f }, w_info.texture);
	registry.renderRequests.get(weapon_icon_e).used_render_layer = RENDER_LAYER::GAME_MENU;
	registry.motions.get(weapon_icon_e).scale = {308.f, 308.f};
	page_entities.push_back(weapon_icon_e);

	// Create weapon price
	Entity weapon_price_e = createText(renderer, "Price: " + std::to_string(w_info.weapon_price) + "G", { 354.f, 420.f + 154.f + 1.25f * renderer->default_font_size }, 1.25f, COLOR_GREEN, TextAlignment::CENTER);
	registry.renderRequests.emplace(weapon_price_e).used_render_layer = RENDER_LAYER::GAME_MENU;
	page_entities.push_back(weapon_price_e);

	// Create the weapon title
	Entity weapon_name_e = createText(renderer, w_info.weapon_name, {554.f, 280.f}, 2.25f, COLOR_GREEN, TextAlignment::LEFT);
	registry.renderRequests.emplace(weapon_name_e).used_render_layer = RENDER_LAYER::GAME_MENU;
	page_entities.push_back(weapon_name_e);

	// Create weapon stats
	vec2 top_left_stats = { 554, 374 };

	createStatText("Damage: ", std::to_string(w_stat.damage), top_left_stats);
	top_left_stats += vec2({0, 0.75f * renderer->default_font_size + 10.f});
	createStatText("Lifetime: " , std::to_string(w_stat.lifetime), top_left_stats);
	top_left_stats += vec2({ 0, 0.75f * renderer->default_font_size + 10.f});
	createStatText("Magazine size: ", std::to_string(w_stat.magazine_size), top_left_stats);
	top_left_stats += vec2({ 0, 0.75f * renderer->default_font_size + 10.f});
	createStatText("Reload time: ", std::to_string(w_stat.reload_time), top_left_stats);
	top_left_stats += vec2({ 0, 0.75f * renderer->default_font_size + 10.f});
	createStatText("Fire rate: ", std::to_string(w_stat.fire_rate), top_left_stats);

	// Create the buy button
	Entity buy_btn_e = createButton(renderer, { 348.f, window_height_px - 250.f }, { 256.f, 128.f });
	Entity buy_text_e = createText(renderer, "Buy", { 348.f, window_height_px - 250.f }, 1.f, COLOR_GREEN, TextAlignment::CENTER);
	
	bind_button_hover(buy_btn_e, buy_text_e, COLOR_GREEN, COLOR_RED);

	registry.renderRequests.emplace(buy_text_e).used_render_layer = RENDER_LAYER::GAME_MENU;
	
	page_entities.push_back(buy_btn_e);
	page_entities.push_back(buy_text_e);
	
	button_entities.push_back(buy_btn_e);

	Text& buy_text = registry.texts.get(buy_text_e);
	Button& buy_btn = registry.buttons.get(buy_btn_e);
	Player& player = registry.players.components.back();

	if (!is_weapon_locked(weapon_on_sale)) {
		buy_text.content = "Owned";
		buy_btn.disabled = true;
	} else {
		buy_btn.on_click = [w_info]() {
			// Unlock weapon and rerender the weapon page
			unlock_weapon(weapon_on_sale);

			Player& player = registry.players.components.back();

			player.gold_balance -= w_info.weapon_price;
			registry.texts.get(balance_text_e).content = "Gold balance: " + std::to_string(player.gold_balance) + "G";
			render_weapons_page();
		};
	}

	int balance_minus_price = player.gold_balance - w_info.weapon_price;
	std::string after_text_content;
	if (balance_minus_price < 0) {
		after_text_content = "You don't have enough gold. You need " + std::to_string(abs(balance_minus_price)) + "G";
	} else {
		after_text_content = "Remaining balance after : " + std::to_string(balance_minus_price) + "G";
	}

	Entity after_text = createText(
		renderer, after_text_content, 
		{348.f + 128.f + 25.f, window_height_px - 250.f}, 1.f, COLOR_GREEN, TextAlignment::LEFT
	);

	registry.renderRequests.emplace(after_text).used_render_layer = RENDER_LAYER::GAME_MENU;
	page_entities.push_back(after_text);
}

void ShopMenu::render_ammo_page() {
	
}

void ShopMenu::render_misc_page() {
	std::cout << "Render misc" << '\n';
}

int ShopMenu::on_mouse_move(vec2 mouse_position) {
	int cursor = GLFW_ARROW_CURSOR;

	for (int i = 0; i < button_entities.size(); i++) {
		Entity& e = button_entities[i];

		if (!registry.buttons.has(e)) {
			continue;
		}

		Motion& motion = registry.motions.get(e);
		Button& button = registry.buttons.get(e);

		if (
			mouse_position.x <= motion.position.x + motion.scale.x / 2 &&
			mouse_position.x >= motion.position.x - motion.scale.x / 2 &&
			mouse_position.y <= motion.position.y + motion.scale.y / 2 &&
			mouse_position.y >= motion.position.y - motion.scale.y / 2 &&
			!button.disabled
		) {
			cursor = GLFW_HAND_CURSOR;
			button.on_mouse_in();
		} else if (!button.disabled) {
			button.on_mouse_out();
		}
	}
	
	return cursor;
}

int ShopMenu::on_mouse_click(int button, int action, int mods) {
	for (Entity btn_e : button_entities) {
		// assert(registry.buttons.has(btn_e);
		if (!registry.buttons.has(btn_e)) {
			continue;
		}

		Button& button = registry.buttons.get(btn_e);

		if (button.hover) {
			button.on_click();
			break;
		}
	}

	return 0;
}

void ShopMenu::close() {
	// Reset states
	current_page = ITEM_TYPE::WEAPON;

	// Remove all related components
	registry.remove_all_components_of(balance_text_e);

	for (Entity e : text_entities) {
		registry.remove_all_components_of(e);
	}

	for (Entity e : button_entities) {
		registry.remove_all_components_of(e);
	}

	for (Entity e : page_entities) {
		registry.remove_all_components_of(e);
	}

	for (Entity e : list_entities) {
		registry.remove_all_components_of(e);
	}

	unlocked_weapons.clear();
	button_entities.clear();
	text_entities.clear();

	page_entities.clear();
	list_entities.clear();
}