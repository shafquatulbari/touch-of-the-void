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
	//Entity weapon_text;
	//Entity ammo_text;
	//Entity score_text;
public:
	void init(RenderSystem* renderer, Health& player_health, Shield& player_shield, Player& player);
	void reinit(Health& player_health, Shield& player_shield, Player& player);
	void update(Health& player_health, Shield& player_shield, Player& player);
};