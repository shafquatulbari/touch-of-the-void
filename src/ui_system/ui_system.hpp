#pragma once

#include "common/common.hpp"
#include "render_system/render_system.hpp"
#include "world_init/world_init.hpp"

class UISystem
{
private:
	RenderSystem* renderer;
	Entity player_health_text;
	Entity player_max_health_text;
	Entity player_shield_text;
	Entity player_ammo_text;
	Entity score_text;
	Entity multiplier_text;
	Entity weapon_slot_1; // unequipped 
	Entity weapon_slot_2; // equipped
	Entity weapon_slot_3; // unequipped
	Entity weapon_slot_4; // unequipped
	Entity total_ammo_icon;
	Entity current_ammo_icon;
public:
	void init(RenderSystem* renderer, Health& player_health, Shield& player_shield, Player& player, int score, float multiplier);
	void reinit();
	void update(Health& player_health, Shield& player_shield, Player& player, int score, float multiplier, int deltaScore);
};