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
	player_health_text = createText(renderer, healthText, { 150.f, 370.0f }, 1.5f, COLOR_BRIGHT_GREEN);
	int roundedMaxHealth = std::max(0, static_cast<int>(player_health.max_health));
	std::string maxHealthText = "/ " + std::to_string(roundedMaxHealth);
	player_max_health_text = createText(renderer, maxHealthText, { 245.f, 370.0f }, 1.5f, COLOR_DARK_GREEN);

	// shield
	int roundedShield = std::max(0, static_cast<int>(player_shield.current_shield));
	int roundedMaxShield = std::max(0, static_cast<int>(player_shield.max_shield));
	int percentage = (roundedShield / roundedMaxShield) * 100;
	std::string shieldText = std::to_string(percentage) + " %";
	player_shield_text = createText(renderer, shieldText, { 150.f, 285.f }, 1.5f, COLOR_BRIGHT_GREEN);

	// weapona and ammo
	createWeaponEquippedIcon(renderer, { 1620.0f, 360.0f });
	createWeaponUnequippedIcon(renderer, { 1650.0f, 580.0f });
	createIconInfinity(renderer, { 1810.0f, 405.0f });

	// score
	std::string scoreText = std::to_string(score);
	score_text = createText(renderer, scoreText, { 1700.0f, 92.0f }, 1.5f, COLOR_GREEN);
	
	// multiplier
	std::string multiplierText = std::to_string(multiplier);
	multiplierText = multiplierText.substr(0, multiplierText.find(".") + 2);
	multiplier_text = createText(renderer, multiplierText, { 1545.0f, 105.0f }, 1.25f, COLOR_BLACK);

	/*weapon_text = createText(renderer, "Weapon: Machine Gun", { 780.0f, 360.0f }, 1.f, COLOR_GREEN);
	ammo_text = createText(renderer, "Ammo: 30 / 30", { 780.0f, 320.0f }, 0.5f, COLOR_GREEN);
	score_text = createText(renderer, "Score: 0", { 780.0f, 120.0f }, 0.7f, COLOR_GREEN);*/
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

void UISystem::reinit(Health& player_health, Shield& player_shield, Player& player, int score, float multiplier) {

	createStatusHud(renderer);
	//// Update Health
	//int roundedHealth = std::max(0, static_cast<int>(player_health.current_health));
	//std::string healthText = std::to_string(roundedHealth);
	//registry.texts.get(player_health_text).content = healthText;

	//int roundedMaxHealth = std::max(0, static_cast<int>(player_health.max_health));
	//std::string maxHealthText = "/ " + std::to_string(roundedMaxHealth);
	//registry.texts.get(player_max_health_text).content = maxHealthText;

	//// Update Shield
	//int roundedShield = std::max(0, static_cast<int>(player_shield.current_shield));
	//int roundedMaxShield = std::max(0, static_cast<int>(player_shield.max_shield));
	//int percentage = (roundedShield / roundedMaxShield) * 100;
	//std::string shieldText = std::to_string(percentage) + " %";
	//registry.texts.get(player_shield_text).content = shieldText;

	//// Update Score
	//std::string scoreText = std::to_string(score);
	//registry.texts.get(score_text).content = scoreText;

	//// Update Multiplier
	//std::string multiplierText = std::to_string(multiplier);
	//multiplierText = multiplierText.substr(0, multiplierText.find(".") + 2);
}