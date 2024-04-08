#include "weapon_system/weapon_constants.hpp"

// Weapon Stats Mapping (damage, lifetime, magazine_size, reload_time, fire_rate)
std::unordered_map<WeaponType, WeaponStats> weapon_stats = {
	{WeaponType::GATLING_GUN, {20.0f, 600.0f, 100, 2000.0f, 40.0f}},
	{WeaponType::SNIPER, {500.0f, 1000.0f, 1, 1500.0f, 0.0f}},
	{WeaponType::SHOTGUN, {20.0f, 400.0f, 6, 1500.0f, 300.0f}},
	{WeaponType::ROCKET_LAUNCHER, {1000.0f, 3000.0f, 1, 2500.0f, 0.0f}},
	{WeaponType::FLAMETHROWER, {5.0f, 600.0f, 200, 2500.0f, 10.0f}},
	{WeaponType::ENERGY_HALO, {200.0f, 10000.0f, 2, 2500.0f, 300.0f}},
	// Add more weapons as needed
};

// Weapon price mapping (base weapon price, ammo price per ammo
std::unordered_map<WeaponType, WeaponPrice> weapon_pricelist {
	{WeaponType::SNIPER, {5, 1}},
	{WeaponType::SHOTGUN, {3, 1}},
	{WeaponType::ROCKET_LAUNCHER, {10, 2}},
	{WeaponType::FLAMETHROWER, {3, 1}},
	{WeaponType::ENERGY_HALO, {5, 1}}
};
