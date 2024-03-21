#pragma once
#include <components/components.hpp>

// World generator that creates rooms / levels using RNG
class WorldGenerator
{
public:
	// populates the fields of a Room
	Room& populateRoom(Room& room);

	Room& generateStartingRoom(Room& room, Level& level);

	Room& generateNewRoom(std::shared_ptr<Room> room, Level& level);

	WorldGenerator()
	{
	}
};
