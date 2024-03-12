#pragma once

// Internal
#include "audio_manager/audio_manager.hpp"
#include "common/common.hpp"
#include "components/components.hpp"
#include "ecs_registry/ecs_registry.hpp"
#include "render_system/render_system.hpp"
#include "weapon_system/weapon_constants.hpp"
#include "world_init/world_init.hpp"

#include <SDL_mixer.h>
#include <random>

// A system that handles weapons
class WeaponSystem
{
private:
	bool init_reload = false;

	// C++ random number generator
	std::default_random_engine rng;
	std::uniform_real_distribution<float> uniform_dist; // number between 0..1
public:
	void step(float elapsed_ms, RenderSystem* renderer, Entity& player);
	void step_projectile_lifetime(float elapsed_ms);
	void reload_weapon();
	void cycle_weapon(int direction, Player& player);
	void handle_rocket_collision(RenderSystem* renderer, Entity projectile);
};