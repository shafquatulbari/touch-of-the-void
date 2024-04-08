#pragma once

#include "ecs/ecs.hpp"
#include "common/common.hpp"
#include "components/components.hpp"
#include "ui_init/ui_init.hpp"
#include "world_init/world_init.hpp"
#include "weapon_system/weapon_system.hpp"

enum class ITEM_TYPE {
	WEAPON,
	AMMO,
	MISC,
	TYPE_COUNT
};

struct ShopItem {
	ITEM_TYPE type;
	WeaponType weapon;
	int price;
};

class ShopMenu {
private:
	static Entity current_hovered;
	static Entity previous_hovered;
	static bool is_hovering;
	static bool has_state_changed;

	static ITEM_TYPE current_page;

	static void render_weapons_page();
	static void render_ammo_page();
	static void render_misc_page();

public:
	static WeaponType weapon_on_sale;
	static Entity balance_text_e;

	static std::vector<Entity> list_button_entities;
	static std::vector<Entity> list_text_entities;

	static std::vector<Entity> button_entities;
	static std::vector<Entity> text_entities;

	static void init(RenderSystem* renderer, WeaponType weapon_on_sale);

	static bool on_mouse_move(vec2 mouse_position);
	static void on_mouse_click(int button, int action, int mods);

	static void close();
};
