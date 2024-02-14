#include "world_init/world_init.hpp"
#include "ecs_registry/ecs_registry.hpp"

Entity createPlayer(RenderSystem *renderer, vec2 pos)
{
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object
	Mesh &mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

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
	health.value = 100.0f; // player health 

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

	// TODO: Create an enemy

	return entity;
}

Entity createObstacle(RenderSystem *renderer, vec2 position, float health_points)
{
	auto entity = Entity();


	// Store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Setting initial motion values
	Motion& motion = registry.motions.emplace(entity);
	motion.position = position;
	motion.complex = false;
	motion.acceleration_rate = 0.0f;
	motion.deceleration_rate = 0.0f;
	motion.max_velocity = 0.0f;
	motion.scale = vec2({ OBSTACLE_BB_WIDTH, OBSTACLE_BB_HEIGHT });

	Health& health = registry.healths.emplace(entity);
	health.value = health_points; // obstacle health 

	registry.players.emplace(entity);
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::OBSTACLE,
		 EFFECT_ASSET_ID::TEXTURED,
		 GEOMETRY_BUFFER_ID::SPRITE });

	return entity;
}

Entity createBullet(RenderSystem *renderer, vec2 position)
{
	auto entity = Entity();

	// TODO: Create a bullet

	return entity;
}

Entity createLine(vec2 position, vec2 scale)
{
	Entity entity = Entity();

	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	registry.renderRequests.insert(
			entity,
			{TEXTURE_ASSET_ID::TEXTURE_COUNT,
			 EFFECT_ASSET_ID::EGG,
			 GEOMETRY_BUFFER_ID::DEBUG_LINE});

	// Create motion
	Motion &motion = registry.motions.emplace(entity);
	motion.look_angle = 0.f;
	motion.velocity = {0, 0};
	motion.position = position;
	motion.scale = scale;

	registry.debugComponents.emplace(entity);
	return entity;
}

Entity createBackground(RenderSystem *renderer, vec2 position)
{
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object
	Mesh &mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Setting initial motion values
	Motion &motion = registry.motions.emplace(entity);
	motion.position = position;
	motion.scale = vec2({BACKGROUND_BB_WIDTH, BACKGROUND_BB_HEIGHT});

	registry.renderRequests.insert(
			entity,
			{TEXTURE_ASSET_ID::LEVEL1_BACKGROUND,
			 EFFECT_ASSET_ID::TEXTURED,
			 GEOMETRY_BUFFER_ID::SPRITE});

	return Entity();
}

Entity createProjectile(RenderSystem *render, vec2 position, float angle)
{
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object
	Mesh &mesh = render->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// actual firing angle is randomly perturbed based off the accuracy
	// TODO: figure out how to use the rng from the game state
	float accuracy = 0.3f;
	angle += (rand() % 1000 - 500) / 1000.f * accuracy;

	// Setting initial motion values
	Motion &motion = registry.motions.emplace(entity);
	Projectile& projectile = registry.projectiles.emplace(entity);
	motion.position = position;
	motion.look_angle = angle + M_PI / 4;
	motion.scale = vec2({BULLET_BB_WIDTH, BULLET_BB_HEIGHT});
	motion.velocity = vec2({500.0f * cos(angle), 500.0f * sin(angle)});

	// TODO: change the damage value and lifetime into constant variables
	projectile.damage = 10.0f; 
	projectile.lifetime = 1000.0f;

	registry.renderRequests.insert(
			entity,
			{TEXTURE_ASSET_ID::BULLET,
			 EFFECT_ASSET_ID::TEXTURED,
			 GEOMETRY_BUFFER_ID::SPRITE});

	return entity;
}