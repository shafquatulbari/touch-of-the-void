#include "world_init/world_init.hpp"
#include "ecs_registry/ecs_registry.hpp"

Entity createPlayer(RenderSystem* renderer, vec2 pos)
{
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Setting initial motion values
	Motion& motion = registry.motions.emplace(entity);
	motion.position = pos;
	motion.acceleration_rate = 25.0f;
	motion.deceleration_rate = 10.0f;
	motion.max_velocity = 100.0f;
	motion.turn_rate = M_PI/64;
	motion.scale = vec2({ PLAYER_BB_WIDTH, PLAYER_BB_HEIGHT });

	// Create and (empty) Player component
	registry.players.emplace(entity);
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::PLAYER, 
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE });

	return entity;
}

Entity createEnemy(RenderSystem* renderer, vec2 position)
{
	// Reserve en entity
	auto entity = Entity();

	// TODO: Create an enemy

	return entity;
}

Entity createObstacle(RenderSystem* renderer, vec2 position)
{
	auto entity = Entity();

	// TODO: Create an obstacle

	return entity;
}

Entity createBullet(RenderSystem* renderer, vec2 position)
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
		{ TEXTURE_ASSET_ID::TEXTURE_COUNT,
		 EFFECT_ASSET_ID::EGG,
		 GEOMETRY_BUFFER_ID::DEBUG_LINE });

	// Create motion
	Motion& motion = registry.motions.emplace(entity);
	motion.look_angle = 0.f;
	motion.direction_angle = 0.f;
	motion.velocity = 0.f;
	motion.position = position;
	motion.scale = scale;

	registry.debugComponents.emplace(entity);
	return entity;
}