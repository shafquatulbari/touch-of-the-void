#include "ui_system/ui_system.hpp"
#include "render_system/render_system.hpp"
#include "world_init/world_init.hpp"
#include "ecs_registry/ecs_registry.hpp"
#include "audio_manager/audio_manager.hpp"

#include <iomanip>
#include <iostream>

void UISystem::fpsCalculate() {
	//average these many samples to change the frames smoother
	static const int num_samples = 10;
	//buffer, array of the frames
	static float frameTimes[num_samples];
	static int currentFrame = 0;

	static float prevTicks = SDL_GetTicks();

	float currentTicks;
	currentTicks = SDL_GetTicks();

	frameTime = currentTicks - prevTicks;//note: first few ticks will be wrong
	frameTimes[currentFrame % num_samples] = frameTime;

	//need to set ticks again. after current tick is done, it becomes prev ticks
	prevTicks = currentTicks;
	int count;
	//	currentFrame++;

	if (currentFrame < num_samples) {
		count = currentFrame;
	}
	else {
		count = num_samples;
	}

	float averageFrameTime = 0;
	for (int i = 0; i < count; i++) {
		averageFrameTime += frameTimes[i];

	}
	averageFrameTime /= count;

	if (averageFrameTime > 0) {
		fps = 1000.0f / averageFrameTime;
		// ms/s divided by ms/f returns frames/second which is fps
	}
	else {
		//shouldn't ever reach this else case 
		fps = 60.0f;
	}
	currentFrame++;
}

void UISystem::createMap(Level& level) {
	//auto positionhelper = Entity();

	//Motion& helpermotion = registry.motions.emplace(positionhelper);
	//helpermotion.position = { 208.f, 290.f };
	//helpermotion.scale = vec2({ 384.f, 384.f });

	//registry.renderRequests.insert(
	//	positionhelper,
	//	{ TEXTURE_ASSET_ID::MAP_PLACEMENT_HELPER,
	//			 EFFECT_ASSET_ID::TEXTURED,
	//			 GEOMETRY_BUFFER_ID::SPRITE,
	//			RENDER_LAYER::UI });

	
	std::pair<int, int> current_room_coords = level.current_room;
	int max_step = 4;

	auto entity = Entity();

	Motion& motion = registry.motions.emplace(entity);
	motion.position = { 208.f, 290.f };
	motion.scale = vec2({ 36.f, 36.f });

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::CURRENT_ROOM,
						 EFFECT_ASSET_ID::TEXTURED,
						 GEOMETRY_BUFFER_ID::SPRITE,
						RENDER_LAYER::UI });

	for (int y = -max_step; y <= max_step; y++) {
		for (int x = -max_step; x <= max_step; x++) {
			std::pair<int, int> room_coords = std::pair<int, int>(current_room_coords.first + x, current_room_coords.second + y);
			if (room_coords.first == current_room_coords.first && room_coords.second == current_room_coords.second) {
				continue;
			}
			if (level.rooms.find(room_coords) != level.rooms.end()) {
				assert(registry.rooms.has(level.rooms[room_coords]) && "Room does not exist in registry");
				Room& room = registry.rooms.get(level.rooms[room_coords]);
				auto entity = Entity();
				if (room.is_visited) {

					Motion& motion = registry.motions.emplace(entity);
					motion.position = { 208.f + (x * 36.f), 290.f - (y * 36.f) };
					motion.scale = vec2({ 36.f, 36.f });

					registry.renderRequests.insert(
						entity,
						{ TEXTURE_ASSET_ID::CLEARED_ROOM,
											EFFECT_ASSET_ID::TEXTURED,
											GEOMETRY_BUFFER_ID::SPRITE,
										RENDER_LAYER::UI });
				}
				else {
					Motion& motion = registry.motions.emplace(entity);
					motion.position = { 208.f + (x * 36.f), 290.f - (y * 36.f) };
					motion.scale = vec2({ 36.f, 36.f });

					registry.renderRequests.insert(
						entity,
						{ TEXTURE_ASSET_ID::UNVISITED_ROOM,
											EFFECT_ASSET_ID::TEXTURED,
											GEOMETRY_BUFFER_ID::SPRITE,
										RENDER_LAYER::UI });
				}
				drawn_rooms.push_back(entity);
			}
		}
	}
}

void UISystem::updateMap(Level& level) {
	std::pair<int, int> current_room_coords = level.current_room;
	if (current_room == current_room_coords) {
		return;
	}
	else {
		current_room = current_room_coords;
	}
	int max_step = 4;

	// Clear the map
	for (auto entity : drawn_rooms) {
		registry.remove_all_components_of(entity);
	}

	for (int y = -max_step; y <= max_step; y++) {
		for (int x = -max_step; x <= max_step; x++) {
			std::pair<int, int> room_coords = std::pair<int, int>(current_room_coords.first + x, current_room_coords.second + y);
			if (room_coords.first == current_room_coords.first && room_coords.second == current_room_coords.second) {
				continue;
			}
			if (level.rooms.find(room_coords) != level.rooms.end()) {
				assert(registry.rooms.has(level.rooms[room_coords]) && "Room does not exist in registry");
				Room& room = registry.rooms.get(level.rooms[room_coords]);
				auto entity = Entity();

				if (room.is_visited) {
					Motion& motion = registry.motions.emplace(entity);
					motion.position = { 208.f + (x * 36.f), 290.f - (y * 36.f) };
					motion.scale = vec2({ 36.f, 36.f });

					registry.renderRequests.insert(
						entity,
						{ TEXTURE_ASSET_ID::CLEARED_ROOM,
											EFFECT_ASSET_ID::TEXTURED,
											GEOMETRY_BUFFER_ID::SPRITE,
										RENDER_LAYER::UI });
				}
				else {

					Motion& motion = registry.motions.emplace(entity);
					motion.position = { 208.f + (x * 36.f), 290.f - (y * 36.f) };
					motion.scale = vec2({ 36.f, 36.f });

					registry.renderRequests.insert(
						entity,
						{ TEXTURE_ASSET_ID::UNVISITED_ROOM,
											EFFECT_ASSET_ID::TEXTURED,
											GEOMETRY_BUFFER_ID::SPRITE,
										RENDER_LAYER::UI });
				}

				drawn_rooms.push_back(entity);
			}
		}
	}
}	

void UISystem::init(RenderSystem* renderer_arg, Health& player_health, Shield& player_shield, Player& player, int score, float multiplier, Level& current_level)
{
	this->renderer = renderer_arg;
	maxFps = 144.0f;
	// create template box
	createStatusHud(renderer);
	current_room = std::pair<int, int>(0, 0);
	createMap(current_level);

	// health 
	int roundedHealth = std::max(0, static_cast<int>(player_health.current_health));
	std::string healthText = std::to_string(roundedHealth);
	player_health_text = createText(renderer, healthText, { 150.f, 654.0f }, 1.5f, COLOR_BRIGHT_GREEN, TextAlignment::LEFT);
	int roundedMaxHealth = std::max(0, static_cast<int>(player_health.max_health));
	std::string maxHealthText = "/ " + std::to_string(roundedMaxHealth);
	player_max_health_text = createText(renderer, maxHealthText, { 245.f, 654.0f }, 1.5f, COLOR_DARK_GREEN, TextAlignment::LEFT);

	// shield
	int roundedShield = std::max(0, static_cast<int>(player_shield.current_shield));
	int roundedMaxShield = std::max(0, static_cast<int>(player_shield.max_shield));
	int percentage = (int)(((float)roundedShield / (float)roundedMaxShield) * 100.f);
	std::string shieldText = std::to_string(percentage) + " %";
	player_shield_text = createText(renderer, shieldText, { 150.f, 739.f }, 1.5f, COLOR_BRIGHT_GREEN, TextAlignment::LEFT);

	// fps
	fps_text = createText(renderer, "FPS:", { 1760.0f, 30.0f }, 0.8f, { 0.0f, 1.0f, 1.0f }, TextAlignment::LEFT);

	// score
	std::string scoreText = std::to_string(score);
	score_text = createText(renderer, scoreText, { 1750.0f, 886.0f }, 1.5f, COLOR_GREEN, TextAlignment::RIGHT);

	// funds
	std::string fundsText = std::to_string(player.gold_balance);
	funds_text = createText(renderer, fundsText, {1750.f, 932.0f }, 1.5f, COLOR_GREEN, TextAlignment::RIGHT);

	// multiplier
	std::string multiplierText = std::to_string(multiplier);
	multiplierText = multiplierText.substr(0, multiplierText.find(".") + 2);
	multiplier_text = createText(renderer, multiplierText, { 1545.0f, 919.0f }, 1.25f, COLOR_BLACK, TextAlignment::LEFT);
	
	std::string total_ammo_count_text;
	if (player.total_ammo_count[player.weapon_type] == INT_MIN) {
		total_ammo_count_text = "Locked";
	} else {
		total_ammo_count_text = std::to_string(player.total_ammo_count[player.weapon_type]);
	}

	std::string ammo_count_text;
	if (player.total_ammo_count[player.weapon_type] == INT_MIN) {
		total_ammo_count_text = "N/A";
	} else {
		total_ammo_count_text = std::to_string(player.total_ammo_count[player.weapon_type]);
	}

	switch (player.weapon_type) {
	case WeaponType::GATLING_GUN:
		weapon_slot_1 = createWeaponUnequippedIcon(renderer, { 1648.0f, 130.0f }, TEXTURE_ASSET_ID::ENERGY_HALO_UNEQUIPPED); // 1st slot EQUIPPED
		weapon_slot_2 = createWeaponEquippedIcon(renderer, { 1616.0f, 354.0f }, TEXTURE_ASSET_ID::GATLING_GUN_EQUIPPED); // 2nd slot EQUIPPED
		weapon_slot_3 = createWeaponUnequippedIcon(renderer, { 1648.0f, 578.0f }, TEXTURE_ASSET_ID::SNIPER_UNEQUIPPED); // 3rd slot UN-EQUIPPED
		weapon_slot_4 = createWeaponUnequippedIcon(renderer, { 1648.0f, 738.0f }, TEXTURE_ASSET_ID::SHOTGUN_UNEQUIPPED); // 4th slot UN-EQUIPPED
		//total_ammo_icon = createIconInfinity(renderer, { 1810.0f, 405.0f });
		current_ammo_icon = createText(renderer, ammo_count_text, { 1810.0f, 425.0f }, 1.5f, COLOR_BLACK, TextAlignment::CENTER);
		total_ammo_text = createText(renderer, "Infinite", { 1810.0f, 350.0f }, 1.0f, COLOR_BRIGHT_GREEN, TextAlignment::CENTER);
		break;
	case WeaponType::SNIPER:
		weapon_slot_1 = createWeaponUnequippedIcon(renderer, { 1648.0f, 130.0f }, TEXTURE_ASSET_ID::GATLING_GUN_UNEQUIPPED); // 1st slot EQUIPPED
		weapon_slot_2 = createWeaponEquippedIcon(renderer, { 1616.0f, 354.0f }, TEXTURE_ASSET_ID::SNIPER_EQUIPPED); // 2nd slot EQUIPPED
		weapon_slot_3 = createWeaponUnequippedIcon(renderer, { 1648.0f, 578.0f }, TEXTURE_ASSET_ID::SHOTGUN_UNEQUIPPED); // 3rd slot UN-EQUIPPED
		weapon_slot_4 = createWeaponUnequippedIcon(renderer, { 1648.0f, 738.0f }, TEXTURE_ASSET_ID::ROCKET_LAUNCHER_UNEQUIPPED); // 4th slot UN-EQUIPPED
		current_ammo_icon = createText(renderer, ammo_count_text, { 1810.0f, 425.0f }, 1.5f, COLOR_BLACK, TextAlignment::CENTER);
		total_ammo_text = createText(renderer, total_ammo_count_text, { 1810.0f, 350.0f }, 1.0f, COLOR_BRIGHT_GREEN, TextAlignment::CENTER);
		break;
	case WeaponType::SHOTGUN:
		weapon_slot_1 = createWeaponUnequippedIcon(renderer, { 1648.0f, 130.0f }, TEXTURE_ASSET_ID::SNIPER_UNEQUIPPED); // 1st slot EQUIPPED
		weapon_slot_2 = createWeaponEquippedIcon(renderer, { 1616.0f, 354.0f }, TEXTURE_ASSET_ID::SHOTGUN_EQUIPPED); // 2nd slot EQUIPPED
		weapon_slot_3 = createWeaponUnequippedIcon(renderer, { 1648.0f, 578.0f }, TEXTURE_ASSET_ID::ROCKET_LAUNCHER_UNEQUIPPED); // 3rd slot UN-EQUIPPED
		weapon_slot_4 = createWeaponUnequippedIcon(renderer, { 1648.0f, 738.0f }, TEXTURE_ASSET_ID::FLAME_THROWER_UNEQUIPPED); // 4th slot UN-EQUIPPED
		current_ammo_icon = createText(renderer, ammo_count_text, { 1810.0f, 425.0f }, 1.5f, COLOR_BLACK, TextAlignment::CENTER);
		total_ammo_text = createText(renderer, total_ammo_count_text, { 1810.0f, 350.0f }, 1.0f, COLOR_BRIGHT_GREEN, TextAlignment::CENTER);
		break;
	case WeaponType::ROCKET_LAUNCHER:
		weapon_slot_1 = createWeaponUnequippedIcon(renderer, { 1648.0f, 130.0f }, TEXTURE_ASSET_ID::SHOTGUN_UNEQUIPPED); // 1st slot EQUIPPED
		weapon_slot_2 = createWeaponEquippedIcon(renderer, { 1616.0f, 354.0f }, TEXTURE_ASSET_ID::ROCKET_LAUNCHER_EQUIPPED); // 2nd slot EQUIPPED
		weapon_slot_3 = createWeaponUnequippedIcon(renderer, { 1648.0f, 578.0f }, TEXTURE_ASSET_ID::FLAME_THROWER_UNEQUIPPED); // 3rd slot UN-EQUIPPED
		weapon_slot_4 = createWeaponUnequippedIcon(renderer, { 1648.0f, 738.0f }, TEXTURE_ASSET_ID::ENERGY_HALO_UNEQUIPPED); // 4th slot UN-EQUIPPED
		current_ammo_icon = createText(renderer, ammo_count_text, { 1810.0f, 425.0f }, 1.5f, COLOR_BLACK, TextAlignment::CENTER);
		total_ammo_text = createText(renderer, total_ammo_count_text, { 1810.0f, 350.0f }, 1.0f, COLOR_BRIGHT_GREEN, TextAlignment::CENTER);
		break;
	case WeaponType::FLAMETHROWER:
		weapon_slot_1 = createWeaponUnequippedIcon(renderer, { 1648.0f, 130.0f }, TEXTURE_ASSET_ID::ROCKET_LAUNCHER_UNEQUIPPED); // 1st slot EQUIPPED
		weapon_slot_2 = createWeaponEquippedIcon(renderer, { 1616.0f, 354.0f }, TEXTURE_ASSET_ID::FLAME_THROWER_EQUIPPED); // 2nd slot EQUIPPED
		weapon_slot_3 = createWeaponUnequippedIcon(renderer, { 1648.0f, 578.0f }, TEXTURE_ASSET_ID::ENERGY_HALO_UNEQUIPPED); // 3rd slot UN-EQUIPPED
		weapon_slot_4 = createWeaponUnequippedIcon(renderer, { 1648.0f, 738.0f }, TEXTURE_ASSET_ID::GATLING_GUN_UNEQUIPPED); // 4th slot UN-EQUIPPED
		current_ammo_icon = createText(renderer, ammo_count_text, { 1810.0f, 425.0f }, 1.5f, COLOR_BLACK, TextAlignment::CENTER);
		total_ammo_text = createText(renderer, total_ammo_count_text, { 1810.0f, 350.0f }, 1.0f, COLOR_BRIGHT_GREEN, TextAlignment::CENTER);
		break;
	case WeaponType::ENERGY_HALO:
		weapon_slot_1 = createWeaponUnequippedIcon(renderer, { 1648.0f, 130.0f }, TEXTURE_ASSET_ID::FLAME_THROWER_UNEQUIPPED); // 1st slot EQUIPPED
		weapon_slot_2 = createWeaponEquippedIcon(renderer, { 1616.0f, 354.0f }, TEXTURE_ASSET_ID::ENERGY_HALO_EQUIPPED); // 2nd slot EQUIPPED
		weapon_slot_3 = createWeaponUnequippedIcon(renderer, { 1648.0f, 578.0f }, TEXTURE_ASSET_ID::GATLING_GUN_UNEQUIPPED); // 3rd slot UN-EQUIPPED
		weapon_slot_4 = createWeaponUnequippedIcon(renderer, { 1648.0f, 738.0f }, TEXTURE_ASSET_ID::SNIPER_UNEQUIPPED); // 4th slot UN-EQUIPPED
		current_ammo_icon = createText(renderer, ammo_count_text, { 1810.0f, 425.0f }, 1.5f, COLOR_BLACK, TextAlignment::CENTER);
		total_ammo_text = createText(renderer, total_ammo_count_text, { 1810.0f, 350.0f }, 1.0f, COLOR_BRIGHT_GREEN, TextAlignment::CENTER);
		break;
	case WeaponType::TOTAL_WEAPON_TYPES:
		break;
	default:
		break;
	}
}

void UISystem::update(Health& player_health, Shield& player_shield, Player& player, int score, float multiplier, int deltaScore, bool showFPS, Level& level)
{

	// Update Map
	updateMap(level);

	// Update Health
	int roundedHealth = std::max(0, static_cast<int>(player_health.current_health));
	std::string healthText = std::to_string(roundedHealth);
	registry.texts.get(player_health_text).content = healthText;

	int roundedMaxHealth = std::max(0, static_cast<int>(player_health.max_health));
	std::string maxHealthText = "/ " + std::to_string(roundedMaxHealth);
	registry.texts.get(player_max_health_text).content = maxHealthText;

	// Update Shield
	int roundedShield = std::max(0, static_cast<int>(player_shield.current_shield));
	int roundedMaxShield = std::max(0, static_cast<int>(player_shield.max_shield));
	int percentage = (int)(((float)roundedShield / (float)roundedMaxShield) * 100.f);
	std::string shieldText = std::to_string(percentage) + " %";
	registry.texts.get(player_shield_text).content = shieldText;

	// Update Score
	std::string scoreText = std::to_string(score);
	registry.texts.get(score_text).content = scoreText;

	// Update Funds
	std::string fundsText = std::to_string(player.gold_balance);
	registry.texts.get(funds_text).content = fundsText;

	// Update Multiplier
	std::string multiplierText = std::to_string(multiplier);
	multiplierText = multiplierText.substr(0, multiplierText.find(".") + 2);
	registry.texts.get(multiplier_text).content = multiplierText;

	if (showFPS) {
		// FPS
		float startTicks = SDL_GetTicks();
		fpsCalculate();
		static int frameCounter = 0;
		frameCounter++;
		// update fps every 50 frames 
		if (frameCounter == 25) {
			registry.texts.get(fps_text).content = "FPS: " + std::to_string(static_cast<int>(fps));
			frameCounter = 0;
		}
		float frameTicks = SDL_GetTicks() - startTicks;
	}
	else {
		registry.texts.get(fps_text).content = "";
	}
	
	std::string total_ammo_count_text;
	if (player.total_ammo_count[player.weapon_type] == INT_MIN) {
		total_ammo_count_text = "Locked";
	} else {
		total_ammo_count_text = std::to_string(player.total_ammo_count[player.weapon_type]);
	}

	std::string ammo_count_text;
	if (player.ammo_count == INT_MIN) {
		ammo_count_text = "N/A";
	} else {
		ammo_count_text = std::to_string(player.ammo_count);
	}

	switch (player.weapon_type) {
	case WeaponType::GATLING_GUN:
		registry.renderRequests.get(weapon_slot_1).used_texture = TEXTURE_ASSET_ID::ENERGY_HALO_UNEQUIPPED; // 1st slot EQUIPPED
		registry.renderRequests.get(weapon_slot_2).used_texture = TEXTURE_ASSET_ID::GATLING_GUN_EQUIPPED; // 2nd slot EQUIPPED
		registry.renderRequests.get(weapon_slot_3).used_texture = TEXTURE_ASSET_ID::SNIPER_UNEQUIPPED; // 3rd slot UN-EQUIPPED
		registry.renderRequests.get(weapon_slot_4).used_texture = TEXTURE_ASSET_ID::SHOTGUN_UNEQUIPPED; // 4th slot UN-EQUIPPED
		// remove the old ammo icon, and create a new one 
		// NOTE: this is a temporary solution ideally we should sense the change in ammo and update the ammo icon only when the ammo changes
		//registry.remove_all_components_of(total_ammo_icon);
		//total_ammo_icon = createIconInfinity(renderer, { 1810.0f, 405.0f });
		registry.texts.get(current_ammo_icon).content = ammo_count_text;
		registry.texts.get(total_ammo_text).content = "Infinite";
		break;
	case WeaponType::SNIPER:
		registry.renderRequests.get(weapon_slot_1).used_texture = TEXTURE_ASSET_ID::GATLING_GUN_UNEQUIPPED; // 1st slot EQUIPPED
		registry.renderRequests.get(weapon_slot_2).used_texture = TEXTURE_ASSET_ID::SNIPER_EQUIPPED; // 2nd slot EQUIPPED
		registry.renderRequests.get(weapon_slot_3).used_texture = TEXTURE_ASSET_ID::SHOTGUN_UNEQUIPPED; // 3rd slot UN-EQUIPPED
		registry.renderRequests.get(weapon_slot_4).used_texture = TEXTURE_ASSET_ID::ROCKET_LAUNCHER_UNEQUIPPED; // 4th slot UN-EQUIPPED
		registry.texts.get(current_ammo_icon).content = ammo_count_text;
		registry.texts.get(total_ammo_text).content = total_ammo_count_text;
		break;
	case WeaponType::SHOTGUN:
		registry.renderRequests.get(weapon_slot_1).used_texture = TEXTURE_ASSET_ID::SNIPER_UNEQUIPPED; // 1st slot EQUIPPED
		registry.renderRequests.get(weapon_slot_2).used_texture = TEXTURE_ASSET_ID::SHOTGUN_EQUIPPED; // 2nd slot EQUIPPED
		registry.renderRequests.get(weapon_slot_3).used_texture = TEXTURE_ASSET_ID::ROCKET_LAUNCHER_UNEQUIPPED; // 3rd slot UN-EQUIPPED
		registry.renderRequests.get(weapon_slot_4).used_texture = TEXTURE_ASSET_ID::FLAME_THROWER_UNEQUIPPED; // 4th slot UN-EQUIPPED
		registry.texts.get(current_ammo_icon).content = ammo_count_text;
		registry.texts.get(total_ammo_text).content = total_ammo_count_text;
		break;
	case WeaponType::ROCKET_LAUNCHER:
		registry.renderRequests.get(weapon_slot_1).used_texture = TEXTURE_ASSET_ID::SHOTGUN_UNEQUIPPED; // 1st slot EQUIPPED
		registry.renderRequests.get(weapon_slot_2).used_texture = TEXTURE_ASSET_ID::ROCKET_LAUNCHER_EQUIPPED; // 2nd slot EQUIPPED
		registry.renderRequests.get(weapon_slot_3).used_texture = TEXTURE_ASSET_ID::FLAME_THROWER_UNEQUIPPED; // 3rd slot UN-EQUIPPED
		registry.renderRequests.get(weapon_slot_4).used_texture = TEXTURE_ASSET_ID::ENERGY_HALO_UNEQUIPPED; // 4th slot UN-EQUIPPED
		registry.texts.get(current_ammo_icon).content = ammo_count_text;
		registry.texts.get(total_ammo_text).content = total_ammo_count_text;
		break;
	case WeaponType::FLAMETHROWER:
		registry.renderRequests.get(weapon_slot_1).used_texture = TEXTURE_ASSET_ID::ROCKET_LAUNCHER_UNEQUIPPED; // 1st slot EQUIPPED
		registry.renderRequests.get(weapon_slot_2).used_texture = TEXTURE_ASSET_ID::FLAME_THROWER_EQUIPPED; // 2nd slot EQUIPPED
		registry.renderRequests.get(weapon_slot_3).used_texture = TEXTURE_ASSET_ID::ENERGY_HALO_UNEQUIPPED; // 3rd slot UN-EQUIPPED
		registry.renderRequests.get(weapon_slot_4).used_texture = TEXTURE_ASSET_ID::GATLING_GUN_UNEQUIPPED; // 4th slot UN-EQUIPPED
		registry.texts.get(current_ammo_icon).content = ammo_count_text;
		registry.texts.get(total_ammo_text).content = total_ammo_count_text;
		break;
	case WeaponType::ENERGY_HALO:
		registry.renderRequests.get(weapon_slot_1).used_texture = TEXTURE_ASSET_ID::FLAME_THROWER_UNEQUIPPED; // 1st slot EQUIPPED
		registry.renderRequests.get(weapon_slot_2).used_texture = TEXTURE_ASSET_ID::ENERGY_HALO_EQUIPPED; // 2nd slot EQUIPPED
		registry.renderRequests.get(weapon_slot_3).used_texture = TEXTURE_ASSET_ID::GATLING_GUN_UNEQUIPPED; // 3rd slot UN-EQUIPPED
		registry.renderRequests.get(weapon_slot_4).used_texture = TEXTURE_ASSET_ID::SNIPER_UNEQUIPPED; // 4th slot UN-EQUIPPED
		registry.texts.get(current_ammo_icon).content = ammo_count_text;
		registry.texts.get(total_ammo_text).content = total_ammo_count_text;
		break;
	case WeaponType::TOTAL_WEAPON_TYPES:
		break;
	default:
		break;
	}

}

void UISystem::reinit(Health& player_health, Shield& player_shield, Player& player, int score, float multiplier, int deltaScore) {
	createStatusHud(renderer);

	// Update Health
	int roundedHealth = std::max(0, static_cast<int>(player_health.current_health));
	std::string healthText = std::to_string(roundedHealth);
	registry.texts.get(player_health_text).content = healthText;

	int roundedMaxHealth = std::max(0, static_cast<int>(player_health.max_health));
	std::string maxHealthText = "/ " + std::to_string(roundedMaxHealth);
	registry.texts.get(player_max_health_text).content = maxHealthText;

	// Update Shield
	int roundedShield = std::max(0, static_cast<int>(player_shield.current_shield));
	int roundedMaxShield = std::max(0, static_cast<int>(player_shield.max_shield));
	int percentage = (roundedShield / roundedMaxShield) * 100;
	std::string shieldText = std::to_string(percentage) + " %";
	registry.texts.get(player_shield_text).content = shieldText;

	// Update Score
	std::string scoreText = std::to_string(score);
	registry.texts.get(score_text).content = scoreText;

	// Update Funds
	std::string fundsText = std::to_string(player.gold_balance);
	registry.texts.get(funds_text).content = fundsText;

	// Update Multiplier
	std::string multiplierText = std::to_string(multiplier);
	multiplierText = multiplierText.substr(0, multiplierText.find(".") + 2);
	registry.texts.get(multiplier_text).content = multiplierText;

	switch (player.weapon_type) {
	case WeaponType::GATLING_GUN:
		weapon_slot_1 = createWeaponUnequippedIcon(renderer, { 1648.0f, 130.0f }, TEXTURE_ASSET_ID::ENERGY_HALO_UNEQUIPPED); // 1st slot EQUIPPED
		weapon_slot_2 = createWeaponEquippedIcon(renderer, { 1616.0f, 354.0f }, TEXTURE_ASSET_ID::GATLING_GUN_EQUIPPED); // 2nd slot EQUIPPED
		weapon_slot_3 = createWeaponUnequippedIcon(renderer, { 1648.0f, 578.0f }, TEXTURE_ASSET_ID::SNIPER_UNEQUIPPED); // 3rd slot UN-EQUIPPED
		weapon_slot_4 = createWeaponUnequippedIcon(renderer, { 1648.0f, 738.0f }, TEXTURE_ASSET_ID::SHOTGUN_UNEQUIPPED); // 4th slot UN-EQUIPPED
		//total_ammo_icon = createIconInfinity(renderer, { 1810.0f, 405.0f });
		registry.texts.get(current_ammo_icon).content = std::to_string(player.ammo_count);
		break;
	case WeaponType::SNIPER:
		weapon_slot_1 = createWeaponUnequippedIcon(renderer, { 1648.0f, 130.0f }, TEXTURE_ASSET_ID::GATLING_GUN_UNEQUIPPED); // 1st slot EQUIPPED
		weapon_slot_2 = createWeaponEquippedIcon(renderer, { 1616.0f, 354.0f }, TEXTURE_ASSET_ID::SNIPER_EQUIPPED); // 2nd slot EQUIPPED
		weapon_slot_3 = createWeaponUnequippedIcon(renderer, { 1648.0f, 578.0f }, TEXTURE_ASSET_ID::SHOTGUN_UNEQUIPPED); // 3rd slot UN-EQUIPPED
		weapon_slot_4 = createWeaponUnequippedIcon(renderer, { 1648.0f, 738.0f }, TEXTURE_ASSET_ID::ROCKET_LAUNCHER_UNEQUIPPED); // 4th slot UN-EQUIPPED
		registry.texts.get(current_ammo_icon).content = std::to_string(player.ammo_count);
		break;
	case WeaponType::SHOTGUN:
		weapon_slot_1 = createWeaponUnequippedIcon(renderer, { 1648.0f, 130.0f }, TEXTURE_ASSET_ID::SNIPER_UNEQUIPPED); // 1st slot EQUIPPED
		weapon_slot_2 = createWeaponEquippedIcon(renderer, { 1616.0f, 354.0f }, TEXTURE_ASSET_ID::SHOTGUN_EQUIPPED); // 2nd slot EQUIPPED
		weapon_slot_3 = createWeaponUnequippedIcon(renderer, { 1648.0f, 578.0f }, TEXTURE_ASSET_ID::ROCKET_LAUNCHER_UNEQUIPPED); // 3rd slot UN-EQUIPPED
		weapon_slot_4 = createWeaponUnequippedIcon(renderer, { 1648.0f, 738.0f }, TEXTURE_ASSET_ID::FLAME_THROWER_UNEQUIPPED); // 4th slot UN-EQUIPPED
		registry.texts.get(current_ammo_icon).content = std::to_string(player.ammo_count);
		break;
	case WeaponType::ROCKET_LAUNCHER:
		weapon_slot_1 = createWeaponUnequippedIcon(renderer, { 1648.0f, 130.0f }, TEXTURE_ASSET_ID::SHOTGUN_UNEQUIPPED); // 1st slot EQUIPPED
		weapon_slot_2 = createWeaponEquippedIcon(renderer, { 1616.0f, 354.0f }, TEXTURE_ASSET_ID::ROCKET_LAUNCHER_EQUIPPED); // 2nd slot EQUIPPED
		weapon_slot_3 = createWeaponUnequippedIcon(renderer, { 1648.0f, 578.0f }, TEXTURE_ASSET_ID::FLAME_THROWER_UNEQUIPPED); // 3rd slot UN-EQUIPPED
		weapon_slot_4 = createWeaponUnequippedIcon(renderer, { 1648.0f, 738.0f }, TEXTURE_ASSET_ID::ENERGY_HALO_UNEQUIPPED); // 4th slot UN-EQUIPPED
		registry.texts.get(current_ammo_icon).content = std::to_string(player.ammo_count);
		break;
	case WeaponType::FLAMETHROWER:
		weapon_slot_1 = createWeaponUnequippedIcon(renderer, { 1648.0f, 130.0f }, TEXTURE_ASSET_ID::ROCKET_LAUNCHER_UNEQUIPPED); // 1st slot EQUIPPED
		weapon_slot_2 = createWeaponEquippedIcon(renderer, { 1616.0f, 354.0f }, TEXTURE_ASSET_ID::FLAME_THROWER_EQUIPPED); // 2nd slot EQUIPPED
		weapon_slot_3 = createWeaponUnequippedIcon(renderer, { 1648.0f, 578.0f }, TEXTURE_ASSET_ID::ENERGY_HALO_UNEQUIPPED); // 3rd slot UN-EQUIPPED
		weapon_slot_4 = createWeaponUnequippedIcon(renderer, { 1648.0f, 738.0f }, TEXTURE_ASSET_ID::GATLING_GUN_UNEQUIPPED); // 4th slot UN-EQUIPPED
		registry.texts.get(current_ammo_icon).content = std::to_string(player.ammo_count);
		break;
	case WeaponType::ENERGY_HALO:
		// TODO FIX THIS
		weapon_slot_1 = createWeaponUnequippedIcon(renderer, { 1648.0f, 130.0f }, TEXTURE_ASSET_ID::FLAME_THROWER_UNEQUIPPED); // 1st slot EQUIPPED
		weapon_slot_2 = createWeaponEquippedIcon(renderer, { 1616.0f, 354.0f }, TEXTURE_ASSET_ID::ENERGY_HALO_EQUIPPED); // 2nd slot EQUIPPED
		weapon_slot_3 = createWeaponUnequippedIcon(renderer, { 1648.0f, 578.0f }, TEXTURE_ASSET_ID::GATLING_GUN_UNEQUIPPED); // 3rd slot UN-EQUIPPED
		weapon_slot_4 = createWeaponUnequippedIcon(renderer, { 1648.0f, 738.0f }, TEXTURE_ASSET_ID::SNIPER_UNEQUIPPED); // 4th slot UN-EQUIPPED
		registry.texts.get(current_ammo_icon).content = std::to_string(player.ammo_count);
		break;
	case WeaponType::TOTAL_WEAPON_TYPES:
		break;
	default:
		break;
	}
}