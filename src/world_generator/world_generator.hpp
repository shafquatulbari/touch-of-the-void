#pragma once
#include <components/components.hpp>

// World generator that creates rooms / levels using RNG
class WorldGenerator
{
public:
	// populates the fields of a Room
	void populateRoom(Room& room);

	void populateTutorialRoomOne(Room& room);

	void generateTutorialRoomOne(Room& room, Level& level);

	void generateStartingRoom(Room& room, Level& level);

	void generateNewRoom(Room& room, Level& level, bool is_boss_room);

	// populates the fields of a Boss Room
	void populateBossRoom(Room& room);
	WorldGenerator()
	{
	}
};
