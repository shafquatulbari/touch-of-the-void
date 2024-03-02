#include "world_init/world_init.hpp"
#include "ecs_registry/ecs_registry.hpp"

Entity createPlayer(RenderSystem *renderer, vec2 pos)
{
	auto entity = Entity();

	// Setting initial motion values
	Motion &motion = registry.motions.emplace(entity);
	motion.position = pos;
	motion.complex = true;
	motion.acceleration_rate = 25.0f;
	motion.deceleration_rate = 10.0f;
	motion.max_velocity = 200.0f;
	motion.scale = vec2({PLAYER_BB_WIDTH, PLAYER_BB_HEIGHT});

	// Setting initial health values
	Health& health = registry.healths.emplace(entity);
	health.current_health = 100.0f;
	health.max_health = 100.0f;

	// Create and (empty) Player component
	registry.players.emplace(entity);
	registry.renderRequests.insert(
			entity,
			{TEXTURE_ASSET_ID::PLAYER,
			 EFFECT_ASSET_ID::TEXTURED,
			 GEOMETRY_BUFFER_ID::SPRITE});

	
	return entity;
}

Entity createEnemy(RenderSystem *renderer, vec2 position, float health_points)
{
	// Reserve en entity
	auto entity = Entity();

	// Setting initial motion values
	Motion& motion = registry.motions.emplace(entity);
	AI& ai = registry.ais.emplace(entity);
	ai.type = AI::AIType::MELEE;
	ai.state = AI::AIState::ACTIVE;
	motion.position = position;
	motion.complex = false;
	motion.scale = vec2({ ENEMY_BB_WIDTH, ENEMY_BB_HEIGHT });

	Health& health = registry.healths.emplace(entity);
	health.current_health = health_points;
	health.max_health = health_points; 

	Deadly& deadly = registry.deadlies.emplace(entity);
	deadly.damage = 10.0f;

	registry.obstacles.emplace(entity);
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::ENEMY,
		 EFFECT_ASSET_ID::TEXTURED,
		 GEOMETRY_BUFFER_ID::SPRITE });


	return entity;
}

Entity createObstacle(RenderSystem *renderer, vec2 position)
{
	auto entity = Entity();


	// Setting initial motion values
	Motion& motion = registry.motions.emplace(entity);
	motion.position = position;
	motion.complex = false;
	motion.scale = vec2({ OBSTACLE_BB_WIDTH, OBSTACLE_BB_HEIGHT });

	registry.obstacles.emplace(entity);
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::OBSTACLE,
		 EFFECT_ASSET_ID::TEXTURED,
		 GEOMETRY_BUFFER_ID::SPRITE });

	return entity;
}

Entity createBackground(RenderSystem *renderer)
{
	auto entity = Entity();

	// Setting initial motion values
	Motion &motion = registry.motions.emplace(entity);
	motion.position = { window_width_px / 2, window_height_px / 2 };
	motion.scale = vec2({BACKGROUND_BB_WIDTH, BACKGROUND_BB_HEIGHT});

	registry.renderRequests.insert(
			entity,
			{TEXTURE_ASSET_ID::LEVEL1_BACKGROUND,
			 EFFECT_ASSET_ID::TEXTURED,
			 GEOMETRY_BUFFER_ID::SPRITE});

	createRoom(renderer);

	return Entity();
}

Entity createProjectile(RenderSystem* render, vec2 position, float angle, float rng, float fire_length)
{
	auto entity = Entity();

	// actual firing angle is randomly perturbed based off the accuracy and how long the fire button has been held
	float accuracy = clamp(fire_length * 0.0005f, 0.0f, 0.4f);
	angle += (rng - 0.5f) * accuracy;

	// Setting initial motion values
	Motion &motion = registry.motions.emplace(entity);
	Projectile& projectile = registry.projectiles.emplace(entity);
	motion.position = position;
	motion.look_angle = angle + M_PI / 4;
	motion.scale = vec2({BULLET_BB_WIDTH, BULLET_BB_HEIGHT});
	motion.velocity = vec2({500.0f * cos(angle), 500.0f * sin(angle)});

	// TODO: change the damage value and lifetime into constant variables
	Deadly &deadly = registry.deadlies.emplace(entity);
	deadly.damage = 10.0f;
	projectile.lifetime = 1000.0f;

	registry.renderRequests.insert(
			entity,
			{TEXTURE_ASSET_ID::BULLET,
			 EFFECT_ASSET_ID::TEXTURED,
			 GEOMETRY_BUFFER_ID::SPRITE});

	return entity;
}

// TODO: figure out whether invidiual components are smart and whether this should be moved to a separate file
void createWalls(RenderSystem* render)
{
	auto topWall = Entity();
	auto bottomWall = Entity();
	auto leftWall = Entity();
	auto rightWall = Entity();

	float x_mid = window_width_px / 2;
	float y_mid = window_height_px / 2;
	float x_delta = game_window_size_px / 2 - 16;
	float y_delta = game_window_size_px / 2 - 16;
	float x_max = x_mid + x_delta;
	float x_min = x_mid - x_delta;
	float y_max = y_mid + y_delta;
	float y_min = y_mid - y_delta;

	// top wall
	Motion& top_motion = registry.motions.emplace(topWall);
	top_motion.position = vec2({ x_mid, y_min });
	top_motion.scale = vec2({ game_window_size_px - 64, 32 });

	registry.obstacles.emplace(topWall);
	registry.renderRequests.insert(
		topWall,
		{ TEXTURE_ASSET_ID::LEVEL1_FULL_WALL,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE });

	// bottom wall
	Motion& bottom_motion = registry.motions.emplace(bottomWall);
	bottom_motion.position = vec2({ x_mid, y_max });
	bottom_motion.scale = vec2({ game_window_size_px - 64, -32 });

	registry.obstacles.emplace(bottomWall);
	registry.renderRequests.insert(
		bottomWall,
		{ TEXTURE_ASSET_ID::LEVEL1_FULL_WALL,
					EFFECT_ASSET_ID::TEXTURED,
					GEOMETRY_BUFFER_ID::SPRITE });

	// left wall
	Motion& left_motion = registry.motions.emplace(leftWall);
	left_motion.position = vec2({ x_min, y_mid });
	left_motion.look_angle = M_PI / 2;
	left_motion.scale = vec2({ game_window_size_px - 64, -32 });

	registry.obstacles.emplace(leftWall);
	registry.renderRequests.insert(
		leftWall,
		{ TEXTURE_ASSET_ID::LEVEL1_FULL_WALL,
							EFFECT_ASSET_ID::TEXTURED,
							GEOMETRY_BUFFER_ID::SPRITE });

	// right wall
	Motion& right_motion = registry.motions.emplace(rightWall);
	right_motion.position = vec2({ x_max, y_mid });
	right_motion.look_angle = M_PI / 2;
	right_motion.scale = vec2({ game_window_size_px - 64, 32 });

	registry.obstacles.emplace(rightWall);
	registry.renderRequests.insert(
		rightWall,
		{ TEXTURE_ASSET_ID::LEVEL1_FULL_WALL,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE });

	// corners
	auto topLeftWall = Entity();
	auto topRightWall = Entity();
	auto bottomLeftWall = Entity();
	auto bottomRightWall = Entity();

	// top left wall
	Motion& topLeft_motion = registry.motions.emplace(topLeftWall);
	topLeft_motion.position = vec2({ x_min, y_min });
	topLeft_motion.scale = vec2({ -32, 32 });

	registry.obstacles.emplace(topLeftWall);
	registry.renderRequests.insert(
		topLeftWall,
		{ TEXTURE_ASSET_ID::LEVEL1_WALL_TOP_CORNER,
					EFFECT_ASSET_ID::TEXTURED,
					GEOMETRY_BUFFER_ID::SPRITE });

	// top right wall
	Motion& topRight_motion = registry.motions.emplace(topRightWall);
	topRight_motion.position = vec2({ x_max, y_min });
	topRight_motion.scale = vec2({ 32, 32 });

	registry.obstacles.emplace(topRightWall);
	registry.renderRequests.insert(
		topRightWall,
		{ TEXTURE_ASSET_ID::LEVEL1_WALL_TOP_CORNER,
							EFFECT_ASSET_ID::TEXTURED,
							GEOMETRY_BUFFER_ID::SPRITE });

	// bottom left wall
	Motion& bottomLeft_motion = registry.motions.emplace(bottomLeftWall);
	bottomLeft_motion.position = vec2({ x_min, y_max });
	bottomLeft_motion.scale = vec2({ -32, 32 });

	registry.obstacles.emplace(bottomLeftWall);
	registry.renderRequests.insert(
		bottomLeftWall,
		{ TEXTURE_ASSET_ID::LEVEL1_WALL_BOTTOM_CORNER,
							EFFECT_ASSET_ID::TEXTURED,
							GEOMETRY_BUFFER_ID::SPRITE });

	// bottom right wall
	Motion& bottomRight_motion = registry.motions.emplace(bottomRightWall);
	bottomRight_motion.position = vec2({ x_max, y_max });
	bottomRight_motion.scale = vec2({ 32, 32 });

	registry.obstacles.emplace(bottomRightWall);
	registry.renderRequests.insert(
		bottomRightWall,
		{ TEXTURE_ASSET_ID::LEVEL1_WALL_BOTTOM_CORNER,
									EFFECT_ASSET_ID::TEXTURED,
									GEOMETRY_BUFFER_ID::SPRITE });

}

Entity createRoom(RenderSystem* render)
{
	auto entity = Entity();

	// A wall is four walls on the edges of the game play screen
	// The walls are obstacles

	Room& room = registry.rooms.emplace(entity);
	// TODO: Generate room info randomly
	// world_generator.generateRoom(Room& room, float rng);
	room.is_cleared = true;
	room.obstacle_count = 10;
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

	float x_origin = (window_width_px / 2) - (game_window_size_px / 2) + 16;
	float y_origin = (window_height_px / 2) - (game_window_size_px / 2) + 16;

	for (auto& pos : room.obstacle_positions)
	{
		float x = x_origin + pos.x * game_window_block_size;
		float y = y_origin + pos.y * game_window_block_size;
		createObstacle(render, vec2(x, y));
	}

	for (auto& pos : room.enemy_positions)
	{
		float x = x_origin + pos.x * game_window_block_size;
		float y = y_origin + pos.y * game_window_block_size;
		createEnemy(render, vec2(x, y), 500.0f);
	}

	createWalls(render);

	return entity;
}