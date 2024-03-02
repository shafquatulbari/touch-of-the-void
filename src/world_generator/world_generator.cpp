#include "world_generator/world_generator.hpp"

Room& WorldGenerator::generateRoom(Room& room, float rng)
{
	// space is effectively 15x15 since 480/32 = 15 
	room.is_cleared = true;
	room.obstacle_count = 10;
	room.obstacle_positions = {};
	for (int i = 0; i < room.obstacle_count; i++) {

	}
	room.obstacle_positions = {
		vec2(1,1),
		vec2(2,2),
		vec2(3,3),
		vec2(4,4),
		vec2(5,5),
		vec2(9,9),
		vec2(10,10),
		vec2(11,11),
		vec2(12,12),
		vec2(13,13)
	};

	room.enemy_count = 3;
	room.enemy_positions = {
		vec2(3,6),
		vec2(2,9),
		vec2(12,4)
	};

	return room;
}