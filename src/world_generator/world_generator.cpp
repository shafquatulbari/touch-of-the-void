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
	// space is effectively 30x30 since 960/32 = 30 
	room.is_cleared = false;
	
	std::default_random_engine rng = std::default_random_engine(std::random_device()());

	// number between 2..12 (don't spawn obstacles too close to where we start)
	std::uniform_real_distribution<float> position_uniform_dist(2,12);

	// randomize number of enemies/obstacles per room
	std::uniform_real_distribution<float> num_positions_uniform_dist(3, 6);
	room.obstacle_count = num_positions_uniform_dist(rng);
	room.enemy_count = num_positions_uniform_dist(rng);
	

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
	std::cout << "Generating Starting Room " << std::endl;
	// starting room always has doors on all sides
	room.is_cleared = true;
	room.is_visited = true;

	room.has_left_door = true;
	room.has_right_door = true;
	room.has_top_door = true;
	room.has_bottom_door = true;

	populateRoom(room);
	std::cout << "Room # of Enemies " << room.enemy_count << std::endl;
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
	std::cout << "Generating Room Left of Starting Room " << std::addressof(left_room) << std::endl;
	level.rooms.emplace(std::pair<int, int>(1, 0), right_room);
	std::cout << "Generating Room Right of Starting Room " << std::addressof(right_room) << std::endl;

	level.rooms.emplace(std::pair<int, int>(0, 1), top_room);
	std::cout << "Generating Room Top of Starting Room " << std::addressof(top_room) << std::endl;

	level.rooms.emplace(std::pair<int, int>(0, -1), bottom_room);
	std::cout << "Generating Room Bottom of Starting Room " << std::addressof(bottom_room) << std::endl;

	
}

void WorldGenerator::generateNewRoom(Room& room, Level& level)
{
	std::cout << "1 room.obstacle_positions.size(): " << room.obstacle_positions.size() << std::endl;
	std::cout << "1 room.enemy_positions.size(): " << room.enemy_positions.size() << std::endl;
	std::cout << "1 level.rooms.size(): " << level.rooms.size() << std::endl;

	// find neighbours if they exist, if they don't coin flip to see if we should generate a new room
	// if we should, generate a new room and add it to the level
	std::pair<int, int> current_room_coords = level.current_room;
	std::pair<int, int> left_room_coords = std::pair<int, int>(current_room_coords.first - 1, current_room_coords.second);
	std::pair<int, int> right_room_coords = std::pair<int, int>(current_room_coords.first + 1, current_room_coords.second);
	std::pair<int, int> top_room_coords = std::pair<int, int>(current_room_coords.first, current_room_coords.second + 1);
	std::pair<int, int> bottom_room_coords = std::pair<int, int>(current_room_coords.first, current_room_coords.second - 1);

	Entity current_room_entity = level.rooms[level.current_room];
	Room* current_room_pointer = &room;

	std::cout << "Current room pointer value %p " << current_room_pointer << std::endl;
	/*for (auto& room : registry.rooms.components)
	{
		std::cout << "0 Pointer value of room: " << std::addressof(room) << std::endl;
	}*/
	if (level.rooms.count(left_room_coords) > 0)
	{
		std::cout << "Found left room" << std::endl;
		current_room_pointer->has_left_door = true;
	}
	else {
			// generate a new room
			std::cout << "Generating new left room" << std::endl;
			current_room_pointer->has_left_door = true;
			auto left_room_entity = Entity();
			Room & new_left_room = registry.rooms.emplace(left_room_entity);
			level.rooms.emplace(left_room_coords, left_room_entity);
			std::cout << "New Left Room created! " << "Visited? : " <<  new_left_room.is_visited << std::endl;


			// retrieve current room again (old reference destroyed at this point)
			current_room_pointer = &registry.rooms.get(current_room_entity);
	}
	//for (auto& room: registry.rooms.components)
	//{
	//	std::cout << "1 Pointer value of room: " << std::addressof(room) << std::endl;
	//}
	Entity e = level.rooms[level.current_room];
	if (registry.rooms.has(e)) {
		std::cout << "Entity exists" << std::endl;
	}
	else {
		std::cout << "Entity does not exist" << std::endl;
	}
	std::cout << "Current room pointer value %p " << current_room_pointer << std::endl;


	//std::cout << "2 room.obstacle_positions.size(): " << room.obstacle_positions.size() << std::endl;
	//std::cout << "2 room.enemy_positions.size(): " << room.enemy_positions.size() << std::endl;
	//std::cout << "2 level.rooms.size(): " << level.rooms.size() << std::endl;


	if (level.rooms.count(right_room_coords) > 0)
	{
		std::cout << "Found right room" << std::endl;
		current_room_pointer->has_right_door = true;
	}
	else {
			// generate a new room
			std::cout << "Generating new right room" << std::endl;
			current_room_pointer->has_right_door = true;
			auto right_room_entity = Entity();
			registry.rooms.emplace(right_room_entity);
			level.rooms.emplace(right_room_coords, right_room_entity);

			// retrieve current room again (old reference destroyed at this point)
			current_room_pointer = &registry.rooms.get(current_room_entity);
	}

	std::cout << "Current room pointer value %p " << current_room_pointer << std::endl;


	if (level.rooms.count(top_room_coords) > 0)
	{
		std::cout << "Found top room" << std::endl;
		current_room_pointer->has_top_door = true;
	}
	else {
			// generate a new room
			std::cout << "Generating new top room" << std::endl;
			current_room_pointer->has_top_door = true;
			auto top_room_entity = Entity();
			Room& new_top_room = registry.rooms.emplace(top_room_entity);
			level.rooms.emplace(top_room_coords, top_room_entity);
			std::cout << "New Top Room created! " << "Visited? : " << new_top_room.is_visited << std::endl;

			// retrieve current room again (old reference destroyed at this point)
			current_room_pointer = &registry.rooms.get(current_room_entity);
	}

	std::cout << "Current room pointer value %p " << current_room_pointer << std::endl;

	if (level.rooms.count(bottom_room_coords) > 0)
	{
		std::cout << "Found bottom room" << std::endl;
		current_room_pointer->has_bottom_door = true;
	}
	else {
		// generate a new room
		std::cout << "Generating new bottom room" << std::endl;
		current_room_pointer->has_bottom_door = true;
		auto bottom_room_entity = Entity();
		Room& new_bottom_room = registry.rooms.emplace(bottom_room_entity);
		level.rooms.emplace(bottom_room_coords, bottom_room_entity);
		std::cout << "New Bottom Room created! " << "Visited? : " << new_bottom_room.is_visited << std::endl;


		current_room_pointer = &registry.rooms.get(current_room_entity);
	}
	std::cout << "Current room pointer value %p " << current_room_pointer << std::endl;

	Room& current_room = registry.rooms.get(current_room_entity);
	populateRoom(current_room);
	std::cout << "4 room.obstacle_positions.size(): " << current_room.obstacle_positions.size() << std::endl;
	std::cout << "4 room.enemy_positions.size(): " << current_room.enemy_positions.size() << std::endl;
	std::cout << "4 level.rooms.size(): " << level.rooms.size() << std::endl;

}