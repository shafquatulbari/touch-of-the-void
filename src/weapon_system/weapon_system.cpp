// internal
#include "weapon_system/weapon_system.hpp"

void WeaponSystem::step(float elapsed_ms, RenderSystem* renderer, Entity& player) 
{
	step_projectile_lifetime(elapsed_ms);

	Player& p = registry.players.get(player);
	Motion& p_m = registry.motions.get(player);
	
	// Handle reloading
	if (init_reload) {
		init_reload = false;
		//registry.texts.get(ammo_text).content = "Ammo: Reloading...";
		play_sound(reload_start_sound);
		p.is_reloading = true;
	}
	if (p.is_reloading) {
		p.reload_timer_ms -= elapsed_ms;

		if (p.reload_timer_ms < 0) {
			// Reload complete, refill ammo
			p.is_reloading = false;
			p.reload_timer_ms = weapon_stats[p.weapon_type].reload_time;
			p.ammo_count = weapon_stats[p.weapon_type].magazine_size; // Refill ammo after reload
			// registry.texts.get(ammo_text).content = "Ammo: " + std::to_string(player.ammo_count) + " / " + std::to_string(player.magazine_sizes[player.weapon_type]);
			play_sound(reload_end_sound);
		}
	}
	// Handle firing
	if (p.is_firing && !p.is_reloading && p.ammo_count > 0) {
		p.fire_length_ms += elapsed_ms;
		if (p.fire_rate_timer_ms <= 0) {
			p.fire_rate_timer_ms = weapon_stats[p.weapon_type].fire_rate;
			switch (p.weapon_type)
			{
			case WeaponType::GATLING_GUN:
				createProjectile(renderer, p_m.position, p_m.look_angle - M_PI / 2, uniform_dist(rng), p.fire_length_ms, player);
				play_sound(gatling_gun_sound);
				break;

			case WeaponType::SNIPER:
				createSniperProjectile(renderer, p_m.position, p_m.look_angle - M_PI / 2, uniform_dist(rng), p.fire_length_ms, player);
				play_sound(sniper_sound);
				break;

			case WeaponType::SHOTGUN:
				for (int i = 0; i < 10; i++) {
					createShotgunProjectile(renderer, p_m.position, p_m.look_angle - M_PI / 2, uniform_dist(rng), p.fire_length_ms, i, player);
				}
				play_sound(shotgun_sound);
				break;

			case WeaponType::ROCKET_LAUNCHER:
				createRocketProjectile(renderer, p_m.position, p_m.look_angle - M_PI / 2, uniform_dist(rng), p.fire_length_ms, player);
				play_sound(rocket_launcher_sound);
				break;

			default:
				// Handle an unknown weapon type (should never reach here, hopefully...)
				break;
			}
			p.ammo_count -= 1;
			// registry.texts.get(ammo_text).content = "Ammo: " + std::to_string(p.ammo_count) + " / " + std::to_string(p.magazine_sizes[p.weapon_type]);

			if (p.ammo_count <= 0) {
				reload_weapon();
			}
		}
	}
	if (p.is_firing && !p.is_reloading && p.ammo_count <= 0) {
		reload_weapon();
	}
	p.fire_rate_timer_ms -= elapsed_ms;
}

void WeaponSystem::step_projectile_lifetime(float elapsed_ms) 
{
	for (auto e : registry.projectiles.entities) {
		registry.projectiles.get(e).lifetime -= elapsed_ms;
		if (registry.projectiles.get(e).lifetime <= 0.0f) {
			registry.remove_all_components_of(e);
		}
	}
}

void WeaponSystem::reload_weapon() 
{
	init_reload = true;
}

// Function to cycle player weapons (-1 for previous, 1 for next)
void WeaponSystem::cycle_weapon(int direction, Player& player) 
{
	// Get the current player's weapon type
	WeaponType currentWeapon = player.weapon_type;

	// Store the current ammo count
	player.magazine_ammo_count[player.weapon_type] = player.ammo_count;

	// Get the total number of weapon types
	int numWeapons = static_cast<int>(WeaponType::TOTAL_WEAPON_TYPES);

	// Calculate the new weapon index
	int newWeaponIndex = static_cast<int>(currentWeapon) + direction;
	if (newWeaponIndex < 0) {
		newWeaponIndex = numWeapons - 1;  // Wrap around to the last weapon
	}
	else if (newWeaponIndex >= numWeapons) {
		newWeaponIndex = 0;  // Wrap around to the first weapon
	}

	// Set the new weapon type
	WeaponType newWeapon = static_cast<WeaponType>(newWeaponIndex);
	player.weapon_type = newWeapon;

	// Convert the enum value to a string for printing
	std::string weaponString;
	switch (newWeapon) {
	case WeaponType::GATLING_GUN:
		weaponString = "Weapon: Gatling Gun";
		break;
	case WeaponType::SNIPER:
		weaponString = "Weapon: Sniper";
		break;
	case WeaponType::SHOTGUN:
		weaponString = "Weapon: Shotgun";
		break;
	case WeaponType::ROCKET_LAUNCHER:
		weaponString = "Weapon: Rocket Launcher";
		break;
	default:
		weaponString = "Unknown Weapon";
		break;
	}

	// Update ammo counters and reload timers
	player.ammo_count = player.magazine_ammo_count[player.weapon_type];

	play_sound(cycle_weapon_sound);
}

// Handles collisions for rockets, returns how many enemies are killed
void WeaponSystem::handle_rocket_collision(RenderSystem* renderer, Entity projectile)
{
	Deadly& deadly = registry.deadlies.get(projectile);
	vec2 rocket_position = registry.motions.get(projectile).position;

	createExplosion(renderer, rocket_position, 2.0f, false); // Create a BIG explosion
	play_sound(explosion_sound);

	// Look for other enemies in radius and apply damage
	for (Entity e : registry.ais.entities) {
		if (registry.healths.has(e)) {
			vec2 e_position = registry.motions.get(e).position;

			// Calculate the distance between rocket and enemy and apply damage if within
			float distance = glm::distance(rocket_position, e_position);
			if (distance <= EXPLOSION_BB_HEIGHT) {
				registry.healths.get(e).current_health -= deadly.damage;
			}
		}
	}
}
