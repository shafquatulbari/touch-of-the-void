#include "world_generator/world_generator.hpp"
#include <ctime>
#include <random>

#include "ecs_registry/ecs_registry.hpp"

static bool shouldAvoidPosition(vec2 position) {
	if (position.x >= 6 && position.x <= 8 || position.y >= 6 && position.y <= 8) return true;
	return false;
}

void WorldGenerator::populateRoom(Room& room)
{
	// space is effectively 15x15 since 480/32 = 30 
	room.is_cleared = false;
	
	std::default_random_engine rng = std::default_random_engine(std::random_device()());

	// number between 2..12 (don't spawn obstacles too close to where we start)
	std::uniform_real_distribution<float> position_uniform_dist(2,12);

	// randomize number of enemies/obstacles per room
	std::uniform_real_distribution<float> num_positions_uniform_dist(3, 5);
	room.obstacle_count = num_positions_uniform_dist(rng);
	room.enemy_count = num_positions_uniform_dist(rng);


	// Assume only 1 level - add 1 enemy / obstacle for each room the enemy has cleared
	Level& level = registry.levels.get(registry.levels.entities[0]);
	room.obstacle_count += level.num_rooms_cleared;
	room.enemy_count += level.num_rooms_cleared;

	int cur_obstacles_count = 0;

	while (room.obstacle_positions.size() < room.obstacle_count) {
		int rand_x = std::rint(position_uniform_dist(rng));
		int rand_y = std::rint(position_uniform_dist(rng));
		vec2 position = vec2(rand_x, rand_y);
		if (!shouldAvoidPosition(position)) {
			room.obstacle_positions.insert(vec2(rand_x, rand_y));
			room.all_positions.insert(vec2(rand_x, rand_y));
		}
	}

	while (room.enemy_positions.size() < room.enemy_count) {
		int rand_x = std::rint(position_uniform_dist(rng));
		int rand_y = std::rint(position_uniform_dist(rng));
		vec2 position = vec2(rand_x, rand_y);
		// if something is not already in this position and it's not too close to the middle, add it
		if (!room.all_positions.count(position) == 1 && !shouldAvoidPosition(position)) {
			room.enemy_positions.insert(vec2(rand_x, rand_y));
			room.all_positions.insert(vec2(rand_x, rand_y));
		}
		
	}

}

void WorldGenerator::generateStartingRoom(Room& room, Level& level)
{
	room.is_cleared = false;
	room.is_visited = true;

	populateRoom(room);
	// Generate the neigboring rooms
	auto left_room = Entity();
	//room.left_room = left_room;
	auto right_room = Entity();
	//room.right_room = right_room;
	auto top_room = Entity();
	//room.top_room = top_room;
	auto bottom_room = Entity();
	//room.bottom_room = bottom_room;

	registry.rooms.emplace(left_room);
	registry.rooms.emplace(right_room);
	registry.rooms.emplace(top_room);
	registry.rooms.emplace(bottom_room);

	// update the level with the new rooms, starting room is 0,0
	level.rooms.emplace(std::pair<int, int>(-1, 0), left_room);
	level.rooms.emplace(std::pair<int, int>(1, 0), right_room);
	level.rooms.emplace(std::pair<int, int>(0, 1), top_room);
	level.rooms.emplace(std::pair<int, int>(0, -1), bottom_room);
}

void WorldGenerator::generateNewRoom(Room& room, Level& level, bool is_boss_room)
{

	// find neighbours if they exist
	// if we should, generate a new room and add it to the level
	std::pair<int, int> current_room_coords = level.current_room;
	std::pair<int, int> left_room_coords = std::pair<int, int>(current_room_coords.first - 1, current_room_coords.second);
	std::pair<int, int> right_room_coords = std::pair<int, int>(current_room_coords.first + 1, current_room_coords.second);
	std::pair<int, int> top_room_coords = std::pair<int, int>(current_room_coords.first, current_room_coords.second + 1);
	std::pair<int, int> bottom_room_coords = std::pair<int, int>(current_room_coords.first, current_room_coords.second - 1);

	Entity current_room_entity = level.rooms[level.current_room];
	Room* current_room_pointer = &room;


	if (level.rooms.count(left_room_coords) > 0 && registry.rooms.get(level.rooms[left_room_coords]).is_visited)
	{
		/*current_room_pointer->has_left_door = true;*/
		// remove for M3
	}
	else {
			auto left_room_entity = Entity();
			Room & new_left_room = registry.rooms.emplace(left_room_entity);
			level.rooms.emplace(left_room_coords, left_room_entity);
			// retrieve current room again (old reference destroyed at this point)
			current_room_pointer = &registry.rooms.get(current_room_entity);
	}


	if (level.rooms.count(right_room_coords) > 0 && registry.rooms.get(level.rooms[right_room_coords]).is_visited)
	{
		/*current_room_pointer->has_right_door = true;*/
		// remove for M3
	}
	else {
			// generate a new room
			auto right_room_entity = Entity();
			registry.rooms.emplace(right_room_entity);
			level.rooms.emplace(right_room_coords, right_room_entity);

			// retrieve current room again (old reference destroyed at this point)
			current_room_pointer = &registry.rooms.get(current_room_entity);
	}



	if (level.rooms.count(top_room_coords) > 0 && registry.rooms.get(level.rooms[top_room_coords]).is_visited)
	{
		/*current_room_pointer->has_top_door = true;*/
		// remove for M3
	}
	else {
			// generate a new room
			auto top_room_entity = Entity();
			Room& new_top_room = registry.rooms.emplace(top_room_entity);
			level.rooms.emplace(top_room_coords, top_room_entity);
			// retrieve current room again (old reference destroyed at this point)
			current_room_pointer = &registry.rooms.get(current_room_entity);
	}


	if (level.rooms.count(bottom_room_coords) > 0 && registry.rooms.get(level.rooms[bottom_room_coords]).is_visited)
	{
		
		/*current_room_pointer->has_bottom_door = true;*/
		// remove for M3
	}
	else {
		// generate a new room
		auto bottom_room_entity = Entity();
		Room& new_bottom_room = registry.rooms.emplace(bottom_room_entity);
		level.rooms.emplace(bottom_room_coords, bottom_room_entity);
		current_room_pointer = &registry.rooms.get(current_room_entity);
	}

	Room& current_room = registry.rooms.get(current_room_entity);
	if (is_boss_room)
	{
		populateBossRoom(current_room);
		
	} else {
		populateRoom(current_room);
	}
}


void WorldGenerator::populateBossRoom(Room& room)
{
	// space is effectively 15x15 since 480/32 = 30 
	room.is_cleared = false;

	std::default_random_engine rng = std::default_random_engine(std::random_device()());

	// number between 2..12 (don't spawn obstacles too close to where we start)
	std::uniform_real_distribution<float> position_uniform_dist(2, 12);

	// randomize number of enemies/obstacles per room
	std::uniform_real_distribution<float> num_positions_uniform_dist(7, 9);
	room.obstacle_count = num_positions_uniform_dist(rng);
	room.enemy_count = num_positions_uniform_dist(rng);
	
	// Assume only 1 level - add 1 enemy / obstacle for each room the enemy has cleared
	Level& level = registry.levels.get(registry.levels.entities[0]);
	room.obstacle_count += level.num_rooms_cleared;
	room.enemy_count += level.num_rooms_cleared;

	while (room.obstacle_positions.size() < room.obstacle_count) {
		int rand_x = std::rint(position_uniform_dist(rng));
		int rand_y = std::rint(position_uniform_dist(rng));
		vec2 position = vec2(rand_x, rand_y);
		if (!shouldAvoidPosition(position)) {
			room.obstacle_positions.insert(vec2(rand_x, rand_y));
			room.all_positions.insert(vec2(rand_x, rand_y));
		}
	}

	while (room.enemy_positions.size() < room.enemy_count) {
		int rand_x = std::rint(position_uniform_dist(rng));
		int rand_y = std::rint(position_uniform_dist(rng));
		vec2 position = vec2(rand_x, rand_y);
		// if something is not already in this position and it's not too close to the middle, add it
		if (!room.all_positions.count(position) == 1 && !shouldAvoidPosition(position)) {
			room.enemy_positions.insert(vec2(rand_x, rand_y));
			room.all_positions.insert(vec2(rand_x, rand_y));
		}

	}

}