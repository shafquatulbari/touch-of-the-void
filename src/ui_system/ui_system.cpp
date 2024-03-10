#include "ui_system/ui_system.hpp"
#include "render_system/render_system.hpp"
#include "world_init/world_init.hpp"
#include "ecs_registry/ecs_registry.hpp"

#include <iomanip>

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
	createWeaponUnequippedIcon(renderer, { 1648.0f, 130.0f }); // 1st slot EQUIPPED
	createWeaponEquippedIcon(renderer, { 1616.0f, 354.0f }); // 2nd slot EQUIPPED
	createWeaponUnequippedIcon(renderer, { 1648.0f, 578.0f }); // 3rd slot UN-EQUIPPED
	createWeaponUnequippedIcon(renderer, { 1648.0f, 738.0f }); // 4th slot UN-EQUIPPED
	createIconInfinity(renderer, { 1810.0f, 405.0f });

	// score
	std::string scoreText = std::to_string(score);
	score_text = createText(renderer, scoreText, { 1750.0f, 932.0f }, 1.5f, COLOR_GREEN, TextAlignment::RIGHT);
	
	// multiplier
	std::string multiplierText = std::to_string(multiplier);
	multiplierText = multiplierText.substr(0, multiplierText.find(".") + 2);
	multiplier_text = createText(renderer, multiplierText, { 1545.0f, 919.0f }, 1.25f, COLOR_BLACK, TextAlignment::LEFT);
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

}

void UISystem::reinit() {
	createStatusHud(renderer);
}