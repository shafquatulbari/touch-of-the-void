#include "world_generator/world_generator.hpp"
#include <ctime>
#include <random>

static bool shouldAvoidPosition(vec2 position) {
	if (position.x >= 6 && position.x <= 8 || position.y >= 6 && position.y <= 8) return true;
	return false;
}

Room& WorldGenerator::generateRoom(Room& room)
{
	// space is effectively 15x15 since 480/32 = 15 
	room.is_cleared = true;
	room.obstacle_count = 8;
	room.enemy_count = 3;
	//room.obstacle_positions = {};
	//room.all_positions = {};
	std::default_random_engine rng = std::default_random_engine(std::random_device()());

	// number between 1..12 (don't spawn obstacles too close to where we start)
	std::uniform_real_distribution<float> obstacle_uniform_dist(2,12); 

	

	int cur_obstacles_count = 0;

	while (room.obstacle_positions.size() < room.obstacle_count) {
		int rand_x = std::rint(obstacle_uniform_dist(rng));
		int rand_y = std::rint(obstacle_uniform_dist(rng));
		vec2 position = vec2(rand_x, rand_y);
		if (!shouldAvoidPosition(position)) {
			room.obstacle_positions.insert(vec2(rand_x, rand_y));
			room.all_positions.insert(vec2(rand_x, rand_y));
		}
	}

	while (room.enemy_positions.size() < room.enemy_count) {
		int rand_x = std::rint(obstacle_uniform_dist(rng));
		int rand_y = std::rint(obstacle_uniform_dist(rng));
		vec2 position = vec2(rand_x, rand_y);
		// if something is not already in this position and it's not too close to the middle, add it
		if (!room.all_positions.count(position) == 1 && !shouldAvoidPosition(position)) {
			room.enemy_positions.insert(vec2(rand_x, rand_y));
			room.all_positions.insert(vec2(rand_x, rand_y));
		}
		
	}

	return room;
}