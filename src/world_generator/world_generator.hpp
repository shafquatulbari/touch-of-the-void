#pragma once
#include <components/components.hpp>

// World generator that creates rooms / levels using RNG
class WorldGenerator
{
public:
	// populates the fields of a Room
	void populateRoom(Room& room);

	void generateStartingRoom(Room& room, Level& level);

	void generateNewRoom(Room& room, Level& level);

	WorldGenerator()
	{
	}
};
