#include "ui_system/ui_system.hpp"
#include "render_system/render_system.hpp"
#include "world_init/world_init.hpp"
#include "ecs_registry/ecs_registry.hpp"

#include <iomanip>
#include <iostream>

void UISystem::init(RenderSystem* renderer_arg, Health& player_health, Shield& player_shield, Player& player, int score, float multiplier)
{
	this->renderer = renderer_arg;

	// create template box
	createStatusHud(renderer);
	

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
	int percentage = (roundedShield / roundedMaxShield) * 100;
	std::string shieldText = std::to_string(percentage) + " %";
	player_shield_text = createText(renderer, shieldText, { 150.f, 739.f }, 1.5f, COLOR_BRIGHT_GREEN, TextAlignment::LEFT);

	// weapons and ammo
	// TODO: create weapon icons, and ammo vizualization


	// score
	std::string scoreText = std::to_string(score);
	score_text = createText(renderer, scoreText, { 1750.0f, 932.0f }, 1.5f, COLOR_GREEN, TextAlignment::RIGHT);
	
	// multiplier
	std::string multiplierText = std::to_string(multiplier);
	multiplierText = multiplierText.substr(0, multiplierText.find(".") + 2);
	multiplier_text = createText(renderer, multiplierText, { 1545.0f, 919.0f }, 1.25f, COLOR_BLACK, TextAlignment::LEFT);
	
	switch (player.weapon_type) {
	case WeaponType::GATLING_GUN:
		weapon_slot_1 = createWeaponUnequippedIcon(renderer, { 1648.0f, 130.0f }, TEXTURE_ASSET_ID::FLAME_THROWER_UNEQUIPPED); // 1st slot EQUIPPED
		weapon_slot_2 = createWeaponEquippedIcon(renderer, { 1616.0f, 354.0f }, TEXTURE_ASSET_ID::GATLING_GUN_EQUIPPED); // 2nd slot EQUIPPED
		weapon_slot_3 = createWeaponUnequippedIcon(renderer, { 1648.0f, 578.0f }, TEXTURE_ASSET_ID::SNIPER_UNEQUIPPED); // 3rd slot UN-EQUIPPED
		weapon_slot_4 = createWeaponUnequippedIcon(renderer, { 1648.0f, 738.0f }, TEXTURE_ASSET_ID::SHOTGUN_UNEQUIPPED); // 4th slot UN-EQUIPPED
		//total_ammo_icon = createIconInfinity(renderer, { 1810.0f, 405.0f });
		current_ammo_icon = createText(renderer, std::to_string(player.ammo_count), { 1810.0f, 425.0f }, 1.5f, COLOR_BLACK, TextAlignment::CENTER);
		break;
	case WeaponType::SNIPER:
		weapon_slot_1 = createWeaponUnequippedIcon(renderer, { 1648.0f, 130.0f }, TEXTURE_ASSET_ID::GATLING_GUN_UNEQUIPPED); // 1st slot EQUIPPED
		weapon_slot_2 = createWeaponEquippedIcon(renderer, { 1616.0f, 354.0f }, TEXTURE_ASSET_ID::SNIPER_EQUIPPED); // 2nd slot EQUIPPED
		weapon_slot_3 = createWeaponUnequippedIcon(renderer, { 1648.0f, 578.0f }, TEXTURE_ASSET_ID::SHOTGUN_UNEQUIPPED); // 3rd slot UN-EQUIPPED
		weapon_slot_4 = createWeaponUnequippedIcon(renderer, { 1648.0f, 738.0f }, TEXTURE_ASSET_ID::ROCKET_LAUNCHER_UNEQUIPPED); // 4th slot UN-EQUIPPED
		current_ammo_icon = createText(renderer, std::to_string(player.ammo_count), { 1810.0f, 425.0f }, 1.5f, COLOR_BLACK, TextAlignment::CENTER);
		break;
	case WeaponType::SHOTGUN:
		weapon_slot_1 = createWeaponUnequippedIcon(renderer, { 1648.0f, 130.0f }, TEXTURE_ASSET_ID::SNIPER_UNEQUIPPED); // 1st slot EQUIPPED
		weapon_slot_2 = createWeaponEquippedIcon(renderer, { 1616.0f, 354.0f }, TEXTURE_ASSET_ID::SHOTGUN_EQUIPPED); // 2nd slot EQUIPPED
		weapon_slot_3 = createWeaponUnequippedIcon(renderer, { 1648.0f, 578.0f }, TEXTURE_ASSET_ID::ROCKET_LAUNCHER_UNEQUIPPED); // 3rd slot UN-EQUIPPED
		weapon_slot_4 = createWeaponUnequippedIcon(renderer, { 1648.0f, 738.0f }, TEXTURE_ASSET_ID::FLAME_THROWER_UNEQUIPPED); // 4th slot UN-EQUIPPED
		current_ammo_icon = createText(renderer, std::to_string(player.ammo_count), { 1810.0f, 425.0f }, 1.5f, COLOR_BLACK, TextAlignment::CENTER);
		break;
	case WeaponType::ROCKET_LAUNCHER:
		weapon_slot_1 = createWeaponUnequippedIcon(renderer, { 1648.0f, 130.0f }, TEXTURE_ASSET_ID::SHOTGUN_UNEQUIPPED); // 1st slot EQUIPPED
		weapon_slot_2 = createWeaponEquippedIcon(renderer, { 1616.0f, 354.0f }, TEXTURE_ASSET_ID::ROCKET_LAUNCHER_EQUIPPED); // 2nd slot EQUIPPED
		weapon_slot_3 = createWeaponUnequippedIcon(renderer, { 1648.0f, 578.0f }, TEXTURE_ASSET_ID::FLAME_THROWER_UNEQUIPPED); // 3rd slot UN-EQUIPPED
		weapon_slot_4 = createWeaponUnequippedIcon(renderer, { 1648.0f, 738.0f }, TEXTURE_ASSET_ID::GATLING_GUN_UNEQUIPPED); // 4th slot UN-EQUIPPED
		current_ammo_icon = createText(renderer, std::to_string(player.ammo_count), { 1810.0f, 425.0f }, 1.5f, COLOR_BLACK, TextAlignment::CENTER);
		break;
	case WeaponType::FLAMETHROWER:
		weapon_slot_1 = createWeaponUnequippedIcon(renderer, { 1648.0f, 130.0f }, TEXTURE_ASSET_ID::ROCKET_LAUNCHER_UNEQUIPPED); // 1st slot EQUIPPED
		weapon_slot_2 = createWeaponEquippedIcon(renderer, { 1616.0f, 354.0f }, TEXTURE_ASSET_ID::FLAME_THROWER_EQUIPPED); // 2nd slot EQUIPPED
		weapon_slot_3 = createWeaponUnequippedIcon(renderer, { 1648.0f, 578.0f }, TEXTURE_ASSET_ID::GATLING_GUN_UNEQUIPPED); // 3rd slot UN-EQUIPPED
		weapon_slot_4 = createWeaponUnequippedIcon(renderer, { 1648.0f, 738.0f }, TEXTURE_ASSET_ID::SNIPER_UNEQUIPPED); // 4th slot UN-EQUIPPED
		current_ammo_icon = createText(renderer, std::to_string(player.ammo_count), { 1810.0f, 425.0f }, 1.5f, COLOR_BLACK, TextAlignment::CENTER);
		break;
	case WeaponType::TOTAL_WEAPON_TYPES:
		break;
	default:
		break;
	}
}

void UISystem::update(Health& player_health, Shield& player_shield, Player& player, int score, float multiplier, int deltaScore)
{
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

	// Update Multiplier
	std::string multiplierText = std::to_string(multiplier);
	multiplierText = multiplierText.substr(0, multiplierText.find(".") + 2);
	registry.texts.get(multiplier_text).content = multiplierText;

	switch (player.weapon_type) {
	case WeaponType::GATLING_GUN:
		registry.renderRequests.get(weapon_slot_1).used_texture = TEXTURE_ASSET_ID::FLAME_THROWER_UNEQUIPPED; // 1st slot EQUIPPED
		registry.renderRequests.get(weapon_slot_2).used_texture = TEXTURE_ASSET_ID::GATLING_GUN_EQUIPPED; // 2nd slot EQUIPPED
		registry.renderRequests.get(weapon_slot_3).used_texture = TEXTURE_ASSET_ID::SNIPER_UNEQUIPPED; // 3rd slot UN-EQUIPPED
		registry.renderRequests.get(weapon_slot_4).used_texture = TEXTURE_ASSET_ID::SHOTGUN_UNEQUIPPED; // 4th slot UN-EQUIPPED
		// remove the old ammo icon, and create a new one 
		// NOTE: this is a temporary solution ideally we should sense the change in ammo and update the ammo icon only when the ammo changes
		//registry.remove_all_components_of(total_ammo_icon);
		//total_ammo_icon = createIconInfinity(renderer, { 1810.0f, 405.0f });
		registry.texts.get(current_ammo_icon).content = std::to_string(player.ammo_count);
		break;
	case WeaponType::SNIPER:
		registry.renderRequests.get(weapon_slot_1).used_texture = TEXTURE_ASSET_ID::GATLING_GUN_UNEQUIPPED; // 1st slot EQUIPPED
		registry.renderRequests.get(weapon_slot_2).used_texture = TEXTURE_ASSET_ID::SNIPER_EQUIPPED; // 2nd slot EQUIPPED
		registry.renderRequests.get(weapon_slot_3).used_texture = TEXTURE_ASSET_ID::SHOTGUN_UNEQUIPPED; // 3rd slot UN-EQUIPPED
		registry.renderRequests.get(weapon_slot_4).used_texture = TEXTURE_ASSET_ID::ROCKET_LAUNCHER_UNEQUIPPED; // 4th slot UN-EQUIPPED
		registry.texts.get(current_ammo_icon).content = std::to_string(player.ammo_count);
		break;
	case WeaponType::SHOTGUN:
		registry.renderRequests.get(weapon_slot_1).used_texture = TEXTURE_ASSET_ID::SNIPER_UNEQUIPPED; // 1st slot EQUIPPED
		registry.renderRequests.get(weapon_slot_2).used_texture = TEXTURE_ASSET_ID::SHOTGUN_EQUIPPED; // 2nd slot EQUIPPED
		registry.renderRequests.get(weapon_slot_3).used_texture = TEXTURE_ASSET_ID::ROCKET_LAUNCHER_UNEQUIPPED; // 3rd slot UN-EQUIPPED
		registry.renderRequests.get(weapon_slot_4).used_texture = TEXTURE_ASSET_ID::FLAME_THROWER_UNEQUIPPED; // 4th slot UN-EQUIPPED
		registry.texts.get(current_ammo_icon).content = std::to_string(player.ammo_count);
		break;
	case WeaponType::ROCKET_LAUNCHER:
		registry.renderRequests.get(weapon_slot_1).used_texture = TEXTURE_ASSET_ID::SHOTGUN_UNEQUIPPED; // 1st slot EQUIPPED
		registry.renderRequests.get(weapon_slot_2).used_texture = TEXTURE_ASSET_ID::ROCKET_LAUNCHER_EQUIPPED; // 2nd slot EQUIPPED
		registry.renderRequests.get(weapon_slot_3).used_texture = TEXTURE_ASSET_ID::FLAME_THROWER_UNEQUIPPED; // 3rd slot UN-EQUIPPED
		registry.renderRequests.get(weapon_slot_4).used_texture = TEXTURE_ASSET_ID::GATLING_GUN_UNEQUIPPED; // 4th slot UN-EQUIPPED
		registry.texts.get(current_ammo_icon).content = std::to_string(player.ammo_count);
		break;
	case WeaponType::FLAMETHROWER:
		registry.renderRequests.get(weapon_slot_1).used_texture = TEXTURE_ASSET_ID::ROCKET_LAUNCHER_UNEQUIPPED; // 1st slot EQUIPPED
		registry.renderRequests.get(weapon_slot_2).used_texture = TEXTURE_ASSET_ID::FLAME_THROWER_EQUIPPED; // 2nd slot EQUIPPED
		registry.renderRequests.get(weapon_slot_3).used_texture = TEXTURE_ASSET_ID::GATLING_GUN_UNEQUIPPED; // 3rd slot UN-EQUIPPED
		registry.renderRequests.get(weapon_slot_4).used_texture = TEXTURE_ASSET_ID::SNIPER_UNEQUIPPED; // 4th slot UN-EQUIPPED
		registry.texts.get(current_ammo_icon).content = std::to_string(player.ammo_count);
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

	// Update Multiplier
	std::string multiplierText = std::to_string(multiplier);
	multiplierText = multiplierText.substr(0, multiplierText.find(".") + 2);
	registry.texts.get(multiplier_text).content = multiplierText;

	switch (player.weapon_type) {
	case WeaponType::GATLING_GUN:
		weapon_slot_1 = createWeaponUnequippedIcon(renderer, { 1648.0f, 130.0f }, TEXTURE_ASSET_ID::FLAME_THROWER_UNEQUIPPED); // 1st slot EQUIPPED
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
		weapon_slot_4 = createWeaponUnequippedIcon(renderer, { 1648.0f, 738.0f }, TEXTURE_ASSET_ID::GATLING_GUN_UNEQUIPPED); // 4th slot UN-EQUIPPED
		registry.texts.get(current_ammo_icon).content = std::to_string(player.ammo_count);
		break;
	case WeaponType::FLAMETHROWER:
		weapon_slot_1 = createWeaponUnequippedIcon(renderer, { 1648.0f, 130.0f }, TEXTURE_ASSET_ID::ROCKET_LAUNCHER_UNEQUIPPED); // 1st slot EQUIPPED
		weapon_slot_2 = createWeaponEquippedIcon(renderer, { 1616.0f, 354.0f }, TEXTURE_ASSET_ID::FLAME_THROWER_EQUIPPED); // 2nd slot EQUIPPED
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