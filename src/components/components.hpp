#pragma once
#include "common/common.hpp"
#include "weapon_system/weapon_constants.hpp"
#include <vector>
#include <unordered_map>
#include "../ext/stb_image/stb_image.h"
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
	Entity left_room;
	Entity right_room;
	Entity top_room;
	Entity bottom_room;
};

// Player component
struct Player
{
	// Store the current ammo count for each weapon
	std::unordered_map<WeaponType, int> magazine_ammo_count = {
		{WeaponType::GATLING_GUN, 100},
		{WeaponType::SNIPER, 1},
		{WeaponType::SHOTGUN, 6},
		{WeaponType::ROCKET_LAUNCHER, 1},
		{WeaponType::FLAMETHROWER, 200}
	};

	bool is_firing = false; // player is currently firing projectiles
	float fire_length_ms = 0.0f; // time the player has been firing
	float fire_rate_timer_ms = 0.0f; // timer for fully-automatic weapons
	WeaponType weapon_type = WeaponType::GATLING_GUN;
	int max_ammo_count;
	int ammo_count;
	bool is_reloading = false; // player is currently reloading and cannot fire
	float reload_timer_ms = 0.0f;

	// if the player is moving between rooms
	bool is_moving_rooms = false;
	// the room that the player is moving to if it is moving rooms
	Entity current_room;

	// Constructor to set the initial values
	Player() : 
		max_ammo_count(weapon_stats[weapon_type].magazine_size),
		ammo_count(max_ammo_count),
		reload_timer_ms(weapon_stats[weapon_type].reload_time) {}
};

// Projectile component
struct Projectile
{
	float lifetime = 0.0f;	// time before the projectile disappears
	Entity source; // New attribute to store the source entity of the projectile
	WeaponType weapon_type;
};

// Burned status timer
struct OnFireTimer
{
	float total_damage = 300.0f; // amount of damage that will be dealt over counter_ms
	float counter_ms = 2000.0f;
	float total_time_ms = 2000.0f;

	Entity fire;
};

// Obstacle component
struct Obstacle
{
	// player can pass through this obstacle (i,e. a door)
	bool is_passable = false;

	// if this is a door, which one
	bool is_top_door = false;
	bool is_bottom_door = false;
	bool is_left_door = false;
	bool is_right_door = false;
};

// No collision check indicator component (for background)
struct NoCollisionCheck 
{

};

struct AI
{
	enum class AIType {MELEE, RANGED};
	AIType type = AIType::MELEE;
	enum class AIState {IDLE, ACTIVE};
	AIState state = AIState::ACTIVE;
	float safe_distance = 150.0f; // the distance that the AI will start behaving from the player
	float shootingCooldown = 0.0f; // time in seconds before the next shot can be made for ranged enemies
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

	vec2 previous_position = {-1, -1};

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

	bool is_passable = false; // if the entity is passable (cannot be collided with)
};

// Stucture to store collision information
struct Collision
{
	// Note, the first object is stored in the ECS container.entities
	Entity other; // the second object involved in the collision
	Collision(Entity& other) { this->other = other; };

	// Scalar of the displacement vector
	float scalar;
};

// Data structure for toggling debug mode
struct Debug 
{
	bool in_debug_mode = 0;
	bool in_freeze_mode = 0;
	bool show_fps = 0;
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

// A timer that will be associated to player moving rooms
struct RoomTransitionTimer
{
	float counter_ms = 0;
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

enum class TextAlignment { LEFT, RIGHT, CENTER };

struct Line 
{
	float width;
	ColoredVertex from;
	ColoredVertex to;
	mat3 trans;
};

struct Text
{
	std::string content;
	vec3 color;
	TextAlignment alignment = TextAlignment::LEFT;
};

// A structure to store the font data of a single character
struct Character {
	unsigned int TextureID;  // ID handle of the glyph texture
	glm::ivec2   Size;       // Size of glyph
	glm::ivec2   Bearing;    // Offset from baseline to left/top of glyph
	unsigned int Advance;    // Offset to advance to next glyph
	char character;
};

// A structure to store the data concerning a single sprite sheet texture
struct Sprite {
	unsigned int TextureID;
	vec2 minTexCoords;
	vec2 maxTexCoords;
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
	INFINITY_AMMO = ENEMY_SPITTER + 1,
	LEVEL1_BACKGROUND = INFINITY_AMMO + 1,
	LEVEL1_DOORS = LEVEL1_BACKGROUND + 1,

	// Full wall closed door textures
	TOP_LEVEL1_FULL_WALL_CLOSED_DOOR = LEVEL1_DOORS + 1,
	RIGHT_LEVEL1_FULL_WALL_CLOSED_DOOR = TOP_LEVEL1_FULL_WALL_CLOSED_DOOR + 1,
	BOTTOM_LEVEL1_FULL_WALL_CLOSED_DOOR = RIGHT_LEVEL1_FULL_WALL_CLOSED_DOOR + 1,
	LEFT_LEVEL1_FULL_WALL_CLOSED_DOOR = BOTTOM_LEVEL1_FULL_WALL_CLOSED_DOOR + 1,

	// Full wall open door textures
	TOP_LEVEL1_FULL_WALL_OPEN_DOOR = LEFT_LEVEL1_FULL_WALL_CLOSED_DOOR + 1,
	RIGHT_LEVEL1_FULL_WALL_OPEN_DOOR = TOP_LEVEL1_FULL_WALL_OPEN_DOOR + 1,
	BOTTOM_LEVEL1_FULL_WALL_OPEN_DOOR = RIGHT_LEVEL1_FULL_WALL_OPEN_DOOR + 1,
	LEFT_LEVEL1_FULL_WALL_OPEN_DOOR = BOTTOM_LEVEL1_FULL_WALL_OPEN_DOOR + 1,
	
	LEVEL1_FULL_WALL_NO_DOOR = LEFT_LEVEL1_FULL_WALL_OPEN_DOOR + 1,

	LEVEL1_OBSTACLE = LEVEL1_FULL_WALL_NO_DOOR + 1,

	LEVEL1_WALL_BOTTOM_CORNER = LEVEL1_OBSTACLE + 1,
	LEVEL1_WALL_END = LEVEL1_WALL_BOTTOM_CORNER + 1,
	LEVEL1_WALL_TOP_CORNER = LEVEL1_WALL_END + 1,
	LEVEL1_WALL = LEVEL1_WALL_TOP_CORNER + 1,

	PLAYER_STATUS_HUD = LEVEL1_WALL + 1,
	PLAYER = PLAYER_STATUS_HUD + 1,

	FLAME_THROWER_EQUIPPED = PLAYER + 1,
	FLAME_THROWER_UNEQUIPPED = FLAME_THROWER_EQUIPPED + 1,
	GATLING_GUN_EQUIPPED = FLAME_THROWER_UNEQUIPPED + 1,
	GATLING_GUN_UNEQUIPPED = GATLING_GUN_EQUIPPED + 1,
	ROCKET_LAUNCHER_EQUIPPED = GATLING_GUN_UNEQUIPPED + 1,
	ROCKET_LAUNCHER_UNEQUIPPED = ROCKET_LAUNCHER_EQUIPPED + 1,
	SHOTGUN_EQUIPPED = ROCKET_LAUNCHER_UNEQUIPPED + 1,
	SHOTGUN_UNEQUIPPED = SHOTGUN_EQUIPPED + 1,
	SNIPER_EQUIPPED = SHOTGUN_UNEQUIPPED + 1,
	SNIPER_UNEQUIPPED = SNIPER_EQUIPPED + 1,

	TEXTURE_COUNT = SNIPER_UNEQUIPPED + 1
};
const int texture_count = (int)TEXTURE_ASSET_ID::TEXTURE_COUNT;

enum class FONT_ASSET_ID {
	VERMIN_VIBES_1989 = 0,
	FONT_COUNT = VERMIN_VIBES_1989 + 1
};
const int font_count = (int)FONT_ASSET_ID::FONT_COUNT;

enum class EFFECT_ASSET_ID {
	COLOURED = 0,
	TEXTURED = COLOURED + 1,
	POST_PROCESS = TEXTURED + 1,
	LINE = POST_PROCESS + 1,
	EFFECT_COUNT = LINE + 1
};
const int effect_count = (int)EFFECT_ASSET_ID::EFFECT_COUNT;

enum class GEOMETRY_BUFFER_ID {
	SPRITE = 0,
	SCREEN_TRIANGLE = SPRITE + 1,
	PLAYER_CH = SCREEN_TRIANGLE + 1,
	BULLET_CH = PLAYER_CH + 1,
	ENEMY_SPITTER_CH = BULLET_CH + 1,
	DEBUG_LINE = ENEMY_SPITTER_CH + 1,
	GEOMETRY_COUNT = DEBUG_LINE + 1
};
const int geometry_count = (int)GEOMETRY_BUFFER_ID::GEOMETRY_COUNT;

enum class SPRITE_SHEET_ID {
	//BLUE_EFFECT = 0,
	EXPLOSION = 0,
	ENEMY_EXPLODER = EXPLOSION + 1,
	FIRE = ENEMY_EXPLODER + 1,
	BULLET_IMPACT = FIRE + 1,
	//GREEN_EFFECT = EXPLOSION + 1,
	//PURPLE_EFFECT = GREEN_EFFECT + 1,
	//RED_EFFECT = PURPLE_EFFECT + 1,
	//YELLOW_EFFECT = RED_EFFECT + 1,
	SPRITE_SHEET_COUNT = BULLET_IMPACT + 1
};
const int sheet_count = (int)SPRITE_SHEET_ID::SPRITE_SHEET_COUNT;

enum class RENDER_LAYER {
	BACKGROUND = 0,
	MIDDLEGROUND = BACKGROUND + 1,
	FOREGROUND = MIDDLEGROUND + 1,
	UI = FOREGROUND + 1,
	RENDER_LAYER_COUNT = UI + 1
};
const int render_layer_count = (int)RENDER_LAYER::RENDER_LAYER_COUNT;

struct RenderRequest {
	TEXTURE_ASSET_ID used_texture = TEXTURE_ASSET_ID::TEXTURE_COUNT;
	EFFECT_ASSET_ID used_effect = EFFECT_ASSET_ID::EFFECT_COUNT;
	GEOMETRY_BUFFER_ID used_geometry = GEOMETRY_BUFFER_ID::GEOMETRY_COUNT;
	RENDER_LAYER used_render_layer = RENDER_LAYER::RENDER_LAYER_COUNT;
};


// A structure to store the data concerning a animation where each frame is a sprite, and the time to display each frame is variable
struct Animation {
	std::vector<std::pair<int, int>> sprites;
	std::vector<float> frame_durations_ms;
	SPRITE_SHEET_ID sheet_id;
	float total_frames;
	float current_frame;
	bool loop;
};

struct AnimationTimer
{
	float counter_ms = 0.0f;
};
