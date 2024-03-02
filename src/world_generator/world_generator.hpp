#pragma once
#include <components/components.hpp>

// World generator that creates rooms / levels using RNG
class WorldGenerator
{
public:
	// creates a random Room entity and adds it to rooms components registry
	Room& generateRoom(Room& room, float rng);

	WorldGenerator()
	{
	}
};
