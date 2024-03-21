#pragma once
#include <components/components.hpp>

// World generator that creates rooms / levels using RNG
class WorldGenerator
{
public:
	// populates the fields of a Room
	void populateRoom(std::shared_ptr<Room> room);

	void generateStartingRoom(std::shared_ptr<Room> room, Level& level);

	void generateNewRoom(std::shared_ptr<Room> room, Level& level);

	WorldGenerator()
	{
	}
};
