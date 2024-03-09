#include "ui_system/ui_system.hpp"
#include "render_system/render_system.hpp"
#include "world_init/world_init.hpp"
#include "ecs_registry/ecs_registry.hpp"

void UISystem::init(RenderSystem* renderer_arg, Health& player_health, Shield& player_shield, Player& player)
{
	this->renderer = renderer_arg;
	createStatusHud(renderer);
	int roundedHealth = std::max(0, static_cast<int>(player_health.current_health));
	std::string healthText = std::to_string(roundedHealth);
	player_health_text = createText(renderer, healthText, { 150.f, 370.0f }, 1.5f, COLOR_BRIGHT_GREEN);
	int roundedMaxHealth = std::max(0, static_cast<int>(player_health.max_health));
	std::string maxHealthText = "/ " + std::to_string(roundedMaxHealth);
	player_max_health_text = createText(renderer, maxHealthText, { 245.f, 370.0f }, 1.5f, COLOR_DARK_GREEN);

	int roundedShield = std::max(0, static_cast<int>(player_shield.current_shield));
	int roundedMaxShield = std::max(0, static_cast<int>(player_shield.max_shield));
	int percentage = (roundedShield / roundedMaxShield) * 100;
	std::string shieldText = std::to_string(percentage) + " %";
	player_shield_text = createText(renderer, shieldText, { 150.f, 285.f }, 1.5f, COLOR_BRIGHT_GREEN);

	/*weapon_text = createText(renderer, "Weapon: Machine Gun", { 780.0f, 360.0f }, 1.f, COLOR_GREEN);
	ammo_text = createText(renderer, "Ammo: 30 / 30", { 780.0f, 320.0f }, 0.5f, COLOR_GREEN);
	score_text = createText(renderer, "Score: 0", { 780.0f, 120.0f }, 0.7f, COLOR_GREEN);*/
}

void UISystem::update(Health& player_health, Shield& player_shield, Player& player)
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

}

void UISystem::reinit(Health& player_health, Shield& player_shield, Player& player) {

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
}