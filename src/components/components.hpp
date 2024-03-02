#pragma once
#include "common/common.hpp"
#include <vector>
#include <unordered_map>
#include "../ext/stb_image/stb_image.h"

// Player component
struct Player
{
	bool is_firing = false; // player is currently firing projectiles
	float fire_length_ms = 0.0f; // time the player has been firing
};

// Obstacle component
struct Obstacle
{
};

// Projectile component
struct Projectile 
{
	float lifetime = 0.0f;	// time before the projectile disappears
};

// Harmful collision component
struct Deadly
{
	float damage = 0.0f; // damage to be dealt to the other entity on collision
};

// Health component 
struct Health
{
	float current_health = 100.0f; // health points of an entity
	float max_health = 100.0f; // maximum health points of an entity
};

// Shield component 
struct Shield
{
	float current_shield = 100.0f; // shield points of an entity
	float max_shield = 100.0f; // maximum shield points of an entity
	float recharge_rate = 0.0f; // rate at which the shield recharges
	float recharge_delay = 0.0f; // delay before the shield starts recharging
};

// All data relevant to the shape and motion of entities
struct Motion {
	vec2 position = { 0, 0 };
	vec2 velocity = { 0, 0 };
	vec2 scale = { 10, 10 };

	float look_angle = 0; // angle the entity is looking at

	bool complex = false; // if the entity has complex motion, i.e. not just a straight line

	bool is_moving_up = false;
	bool is_moving_down = false;
	bool is_moving_left = false;
	bool is_moving_right = false;

	float acceleration_rate = 0.0f;	// amount to be added to velocity if not moving in direction
	float deceleration_rate = 0.0f; // amount to be shaved off velocity if moving in direction
	float max_velocity = 0.0f; 	// maximum velocity in any direction
	float turn_rate = 0.0f;		// how fast the entity can turn
};

struct vec2comp {
	bool operator() (vec2 lhs, vec2 rhs) const
	{
		if (lhs.x < rhs.x) return true;
		if (lhs.x == rhs.x && lhs.y < rhs.y) return true;
		return false;
	}
};
// All data relevant to the contents of a game room
struct Room {
	bool is_cleared = false; // if the room has been cleared of enemies, can contain upgrade

	// The number of enemies in the room
	int enemy_count = 0;

	// the positions of all entities in the room
	std::set<vec2, vec2comp> all_positions;

	// the positions of the enemies in the room
	std::set<vec2, vec2comp> enemy_positions;

	// The number of obstacles in the room
	int obstacle_count = 0;
	// the positions of the obstacles in the room
	std::set<vec2, vec2comp> obstacle_positions;

	// The number of powerups in the room
	int powerup_count = 0;
	// the positions of the powerups in the room
	std::set<vec2, vec2comp> powerup_positions;

	// fields concerning the doors of the room
	bool has_left_door = false;
	bool has_right_door = false;
	bool has_top_door = false;
	bool has_bottom_door = false;

	// neighbouring rooms
	struct Room* left_room;
	struct Room* right_room;
	struct Room* top_room;
	struct Room* bottom_room;
};


struct ReloadTimer
{
	float counter_ms = 0.0f;
};

// Stucture to store collision information
struct Collision
{
	// Note, the first object is stored in the ECS container.entities
	Entity other; // the second object involved in the collision
	Collision(Entity& other) { this->other = other; };
};

// Data structure for toggling debug mode
struct Debug {
	bool in_debug_mode = 0;
	bool in_freeze_mode = 0;
};
extern Debug debugging;

// Sets the brightness of the screen
struct ScreenState
{
	float darken_screen_factor = -1;
};

// A struct to refer to debugging graphics in the ECS
struct DebugComponent
{
	// Note, an empty struct has size 1
};

// A timer that will be associated to dying player
struct DeathTimer
{
	float counter_ms = 3000;
};

// Single Vertex Buffer element for non-textured meshes (coloured.vs.glsl & chicken.vs.glsl)
struct ColoredVertex
{
	vec3 position;
	vec3 color;
};

// Single Vertex Buffer element for textured sprites (textured.vs.glsl)
struct TexturedVertex
{
	vec3 position;
	vec2 texcoord;
};

// Mesh datastructure for storing vertex and index buffers
struct Mesh
{
	static bool loadFromOBJFile(std::string obj_path, std::vector<ColoredVertex>& out_vertices, std::vector<uint16_t>& out_vertex_indices, vec2& out_size);
	vec2 original_size = { 1,1 };
	std::vector<ColoredVertex> vertices;
	std::vector<uint16_t> vertex_indices;
};

/**
 * The following enumerators represent global identifiers refering to graphic
 * assets. For example TEXTURE_ASSET_ID are the identifiers of each texture
 * currently supported by the system.
 *
 * So, instead of referring to a game asset directly, the game logic just
 * uses these enumerators and the RenderRequest struct to inform the renderer
 * how to structure the next draw command.
 *
 * There are 2 reasons for this:
 *
 * First, game assets such as textures and meshes are large and should not be
 * copied around as this wastes memory and runtime. Thus separating the data
 * from its representation makes the system faster.
 *
 * Second, it is good practice to decouple the game logic from the render logic.
 * Imagine, for example, changing from OpenGL to Vulkan, if the game logic
 * depends on OpenGL semantics it will be much harder to do the switch than if
 * the renderer encapsulates all asset data and the game logic is agnostic to it.
 *
 * The final value in each enumeration is both a way to keep track of how many
 * enums there are, and as a default value to represent uninitialized fields.
 */

enum class TEXTURE_ASSET_ID {
	BULLET = 0,
	ENEMY_SPITTER = BULLET + 1,
	LEVEL1_BACKGROUND = ENEMY_SPITTER + 1,
	LEVEL1_DOORS = LEVEL1_BACKGROUND + 1,
	LEVEL1_FULL_WALL_CLOSED_DOOR = LEVEL1_DOORS + 1,
	LEVEL1_FULL_WALL_NO_DOOR = LEVEL1_FULL_WALL_CLOSED_DOOR + 1,
	LEVEL1_FULL_WALL_OPEN_DOOR = LEVEL1_FULL_WALL_NO_DOOR + 1,
	LEVEL1_OBSTACLE = LEVEL1_FULL_WALL_OPEN_DOOR + 1,
	LEVEL1_WALL_BOTTOM_CORNER = LEVEL1_OBSTACLE + 1,
	LEVEL1_WALL_END = LEVEL1_WALL_BOTTOM_CORNER + 1,
	LEVEL1_WALL_TOP_CORNER = LEVEL1_WALL_END + 1,
	LEVEL1_WALL = LEVEL1_WALL_TOP_CORNER + 1,
	PLAYER = LEVEL1_WALL + 1,
	TEXTURE_COUNT = PLAYER + 1
};
const int texture_count = (int)TEXTURE_ASSET_ID::TEXTURE_COUNT;

enum class EFFECT_ASSET_ID {
	COLOURED = 0,
	TEXTURED = COLOURED + 1,
	POST_PROCESS = TEXTURED + 1,
	EFFECT_COUNT = POST_PROCESS + 1
};
const int effect_count = (int)EFFECT_ASSET_ID::EFFECT_COUNT;

enum class GEOMETRY_BUFFER_ID {
	SPRITE = 0,
	SCREEN_TRIANGLE = SPRITE + 1,
	GEOMETRY_COUNT = SCREEN_TRIANGLE + 1
};
const int geometry_count = (int)GEOMETRY_BUFFER_ID::GEOMETRY_COUNT;

struct RenderRequest {
	TEXTURE_ASSET_ID used_texture = TEXTURE_ASSET_ID::TEXTURE_COUNT;
	EFFECT_ASSET_ID used_effect = EFFECT_ASSET_ID::EFFECT_COUNT;
	GEOMETRY_BUFFER_ID used_geometry = GEOMETRY_BUFFER_ID::GEOMETRY_COUNT;
};

