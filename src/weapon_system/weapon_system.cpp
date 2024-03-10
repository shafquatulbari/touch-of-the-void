// internal
#include "weapon_system/weapon_system.hpp"

void WeaponSystem::step(float elapsed_ms, RenderSystem* renderer, Entity& player) {
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
			p.reload_timer_ms = p.reload_times[p.weapon_type];
			p.ammo_count = p.magazine_sizes[p.weapon_type]; // Refill ammo after reload
			// registry.texts.get(ammo_text).content = "Ammo: " + std::to_string(player.ammo_count) + " / " + std::to_string(player.magazine_sizes[player.weapon_type]);
			play_sound(reload_end_sound);
		}
	}
	// Handle firing
	if (p.is_firing && !p.is_reloading && p.ammo_count > 0) {
		p.fire_length_ms += elapsed_ms;
		if (p.fire_rate_timer_ms <= 0) {
			p.fire_rate_timer_ms = p.fire_rates[p.weapon_type];
			switch (p.weapon_type)
			{
			case Player::WeaponType::MACHINE_GUN:
				createProjectile(renderer, p_m.position, p_m.look_angle - M_PI / 2, uniform_dist(rng), p.fire_length_ms, player);
				play_sound(machine_gun_sound);
				break;

			case Player::WeaponType::SNIPER:
				createSniperProjectile(renderer, p_m.position, p_m.look_angle - M_PI / 2, uniform_dist(rng), p.fire_length_ms, player);
				play_sound(sniper_sound);
				break;

			case Player::WeaponType::SHOTGUN:
				for (int i = 0; i < 10; i++) {
					createShotgunProjectile(renderer, p_m.position, p_m.look_angle - M_PI / 2, uniform_dist(rng), p.fire_length_ms, i, player);
				}
				play_sound(shotgun_sound);
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

void WeaponSystem::reload_weapon() {
	init_reload = true;
}

// Function to cycle player weapons (-1 for previous, 1 for next)
void WeaponSystem::cycle_weapon(int direction, Player& player) {

	// Get the current player's weapon type
	Player::WeaponType currentWeapon = player.weapon_type;

	// Store the current ammo count
	player.magazine_ammo_count[player.weapon_type] = player.ammo_count;

	// Get the total number of weapon types
	int numWeapons = static_cast<int>(Player::WeaponType::TOTAL_WEAPON_TYPES);

	// Calculate the new weapon index
	int newWeaponIndex = static_cast<int>(currentWeapon) + direction;
	if (newWeaponIndex < 0) {
		newWeaponIndex = numWeapons - 1;  // Wrap around to the last weapon
	}
	else if (newWeaponIndex >= numWeapons) {
		newWeaponIndex = 0;  // Wrap around to the first weapon
	}

	// Set the new weapon type
	Player::WeaponType newWeapon = static_cast<Player::WeaponType>(newWeaponIndex);
	player.weapon_type = newWeapon;

	// Convert the enum value to a string for printing
	std::string weaponString;
	switch (newWeapon) {
	case Player::WeaponType::MACHINE_GUN:
		weaponString = "Weapon: Machine Gun";
		break;
	case Player::WeaponType::SNIPER:
		weaponString = "Weapon: Sniper";
		break;
	case Player::WeaponType::SHOTGUN:
		weaponString = "Weapon: Shotgun";
		break;
	default:
		weaponString = "Unknown Weapon";
		break;
	}

	// Update ammo counters and reload timers
	player.ammo_count = player.magazine_ammo_count[player.weapon_type];

	play_sound(cycle_weapon_sound);
}