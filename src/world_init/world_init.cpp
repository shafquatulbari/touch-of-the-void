#include "world_init/world_init.hpp"
#include "ecs_registry/ecs_registry.hpp"
#include "weapon_system/weapon_system.hpp"
#include "world_generator/world_generator.hpp"
#include "world_system/world_system.hpp"

#include <cmath>
#include <random>
#include <glm/gtc/random.hpp>


Entity createPlayer(RenderSystem *renderer, vec2 pos)
{
	auto entity = Entity();

	Mesh& p_mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::PLAYER_CH);
	registry.meshPtrs.emplace(entity, &p_mesh);

	// Setting initial motion values
	Motion &motion = registry.motions.emplace(entity);
	motion.position = pos;
	motion.complex = true;
	motion.acceleration_rate = 50.f;
	motion.deceleration_rate = 20.0f;
	motion.max_velocity = 400.0f;
	motion.scale = vec2({PLAYER_BB_WIDTH, PLAYER_BB_HEIGHT});

	Shield& shield = registry.shields.emplace(entity);
	shield.current_shield = 100.0f;
	shield.max_shield = 100.0f;
	shield.recharge_delay = 2000.0f;
	shield.recharge_rate = 1.0f;

	// Setting initial health values
	Health& health = registry.healths.emplace(entity);
	health.current_health = 32.0f;
	health.max_health = 32.0f;

	// Create and (empty) Player component
	registry.players.emplace(entity);
	registry.renderRequests.insert(
			entity,
			{TEXTURE_ASSET_ID::PLAYER,
			 EFFECT_ASSET_ID::TEXTURED,
			 GEOMETRY_BUFFER_ID::SPRITE,
			RENDER_LAYER::FOREGROUND});
	
	return entity;
}

Entity createEnemy(RenderSystem *renderer, vec2 position, float health_points, AI::AIType aiType)
{
	// Reserve en entity
	auto entity = Entity();

	// Setting initial motion values
	Motion& motion = registry.motions.emplace(entity);
	AI& ai = registry.ais.emplace(entity);
	ai.type = aiType; // based on passed parameter
	ai.state = AI::AIState::ACTIVE;
	ai.frequency = 5;
	motion.position = position;
	motion.complex = false;
	motion.scale = vec2({ ENEMY_BB_WIDTH, ENEMY_BB_HEIGHT });

	Health& health = registry.healths.emplace(entity);
	health.current_health = health_points;
	health.max_health = health_points; 

	Deadly& deadly = registry.deadlies.emplace(entity);
	deadly.damage = 10.0f;

	//Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::ENEMY_SPITTER_CH);
	//registry.meshPtrs.emplace(entity, &mesh);

	registry.obstacles.emplace(entity);	
	//registry.obstacles.emplace(entity);
	if (aiType == AI::AIType::MELEE) {
		Animation& animation = registry.animations.emplace(entity);
		animation.sheet_id = SPRITE_SHEET_ID::ENEMY_EXPLODER;
		animation.total_frames = 6;
		animation.current_frame = 0;
		animation.sprites = { {0, 0}, {1, 0}, {2, 0}, {3, 0}, {4, 0}, {5, 0} };
		animation.frame_durations_ms = { 100, 100, 100, 100, 100, 100 };
		animation.loop = true;

		AnimationTimer& animation_timer = registry.animationTimers.emplace(entity);
		animation_timer.counter_ms = animation.frame_durations_ms[0];

		registry.renderRequests.insert(
			entity,
			{ TEXTURE_ASSET_ID::TEXTURE_COUNT,
			 EFFECT_ASSET_ID::TEXTURED,
			 GEOMETRY_BUFFER_ID::SPRITE,
			RENDER_LAYER::FOREGROUND });
	}
	else if (aiType == AI::AIType::TURRET) {
		registry.renderRequests.insert(
			entity,
			{ TEXTURE_ASSET_ID::ENEMY_TURRET_GUN,
			 EFFECT_ASSET_ID::TEXTURED,
			 GEOMETRY_BUFFER_ID::SPRITE,
			RENDER_LAYER::FOREGROUND });

		auto base_entity = Entity();
		Motion& base_motion = registry.motions.emplace(base_entity);
		base_motion.position = position;
		base_motion.scale = vec2({ ENEMY_BB_WIDTH, ENEMY_BB_HEIGHT });
		registry.obstacles.emplace(base_entity);
		registry.noCollisionChecks.emplace(base_entity);
		registry.renderRequests.insert(
			base_entity,
			{ TEXTURE_ASSET_ID::ENEMY_TURRET_BASE,
			 EFFECT_ASSET_ID::TEXTURED,
			 GEOMETRY_BUFFER_ID::SPRITE,
			RENDER_LAYER::MIDDLEGROUND });
	}
	else if (aiType == AI::AIType::RANGED) {
		registry.renderRequests.insert(
			entity,
			{ TEXTURE_ASSET_ID::ENEMY_SPITTER,
			 EFFECT_ASSET_ID::TEXTURED,
			 GEOMETRY_BUFFER_ID::SPRITE,
			RENDER_LAYER::FOREGROUND });
	}
	else if (aiType == AI::AIType::SHOTGUN) {
		Animation& animation = registry.animations.emplace(entity);
		animation.sheet_id = SPRITE_SHEET_ID::ENEMY_SCARAB;
		animation.total_frames = 8;
		animation.current_frame = 0;
		animation.sprites = { {0, 0}, {1, 0}, {2, 0}, {3, 0}, {4, 0}, {5, 0}, {6, 0}, {7, 0} };
		animation.frame_durations_ms = { 50, 50, 50, 50, 50, 50, 50, 50 };
		animation.loop = true;

		AnimationTimer& animation_timer = registry.animationTimers.emplace(entity);
		animation_timer.counter_ms = animation.frame_durations_ms[0];

		registry.renderRequests.insert(
			entity,
			{ TEXTURE_ASSET_ID::TEXTURE_COUNT,
			 EFFECT_ASSET_ID::TEXTURED,
			 GEOMETRY_BUFFER_ID::SPRITE,
			RENDER_LAYER::FOREGROUND });
	}
	else if (aiType == AI::AIType::ROCKET) {
		Animation& animation = registry.animations.emplace(entity);
		animation.sheet_id = SPRITE_SHEET_ID::ENEMY_EXPLODER;
		animation.total_frames = 6;
		animation.current_frame = 0;
		animation.sprites = { {0, 0}, {1, 0}, {2, 0}, {3, 0}, {4, 0}, {5, 0} };
		animation.frame_durations_ms = { 100, 100, 100, 100, 100, 100 };
		animation.loop = true;

		AnimationTimer& animation_timer = registry.animationTimers.emplace(entity);
		animation_timer.counter_ms = animation.frame_durations_ms[0];

		registry.renderRequests.insert(
			entity,
			{ TEXTURE_ASSET_ID::TEXTURE_COUNT,
			 EFFECT_ASSET_ID::TEXTURED,
			 GEOMETRY_BUFFER_ID::SPRITE,
			RENDER_LAYER::FOREGROUND });
	}
	else if (aiType == AI::AIType::FLAMETHROWER) {
		Animation& animation = registry.animations.emplace(entity);
		animation.sheet_id = SPRITE_SHEET_ID::ENEMY_EXPLODER;
		animation.total_frames = 6;
		animation.current_frame = 0;
		animation.sprites = { {0, 0}, {1, 0}, {2, 0}, {3, 0}, {4, 0}, {5, 0} };
		animation.frame_durations_ms = { 100, 100, 100, 100, 100, 100 };
		animation.loop = true;

		AnimationTimer& animation_timer = registry.animationTimers.emplace(entity);
		animation_timer.counter_ms = animation.frame_durations_ms[0];

		registry.renderRequests.insert(
			entity,
			{ TEXTURE_ASSET_ID::TEXTURE_COUNT,
			 EFFECT_ASSET_ID::TEXTURED,
			 GEOMETRY_BUFFER_ID::SPRITE,
			RENDER_LAYER::FOREGROUND });
	}


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
		{ TEXTURE_ASSET_ID::LEVEL1_OBSTACLE,
		 EFFECT_ASSET_ID::TEXTURED,
		 GEOMETRY_BUFFER_ID::SPRITE,
		RENDER_LAYER::MIDDLEGROUND });

	return entity;
}

Entity createBackground(RenderSystem *renderer)
{
	auto entity = Entity();

	// Setting initial motion values
	Motion &motion = registry.motions.emplace(entity);
	motion.position = { window_width_px / 2, window_height_px / 2 };
	motion.scale = vec2({BACKGROUND_BB_WIDTH, BACKGROUND_BB_HEIGHT});

	registry.noCollisionChecks.emplace(entity);

	registry.renderRequests.insert(
			entity,
			{TEXTURE_ASSET_ID::LEVEL1_BACKGROUND,
			 EFFECT_ASSET_ID::TEXTURED,
			 GEOMETRY_BUFFER_ID::SPRITE,
			RENDER_LAYER::BACKGROUND});

	// return the starting room entity
	//return createLevel(renderer);
	return Entity();
}

Entity createProjectile(RenderSystem* render, vec2 position, float angle, float rng, float fire_length, Entity source)
{
	auto entity = Entity();

	// Actual firing angle is randomly perturbed based off the accuracy and how long the fire button has been held
	float accuracy = clamp(fire_length * 0.0005f, 0.0f, 0.4f);
	angle += (rng - 0.5f) * accuracy;

	Mesh& mesh = render->getMesh(GEOMETRY_BUFFER_ID::BULLET_CH);
	registry.meshPtrs.emplace(entity, &mesh);

	// Setting initial motion values
	Motion &motion = registry.motions.emplace(entity);
	Projectile& projectile = registry.projectiles.emplace(entity);
	motion.position = position;
	motion.look_angle = angle + M_PI / 4;
	motion.scale = vec2({BULLET_BB_WIDTH, BULLET_BB_HEIGHT});
	motion.velocity = vec2({1000.0f * cos(angle), 1000.0f * sin(angle)});
	
	// Set the source of the projectile
	registry.projectiles.get(entity).source = source;

	// Set damage and projectile properties
	Deadly& deadly = registry.deadlies.emplace(entity);
	projectile.weapon_type = WeaponType::GATLING_GUN;
	projectile.lifetime = weapon_stats[projectile.weapon_type].lifetime;
	deadly.damage = weapon_stats[projectile.weapon_type].damage;

	registry.renderRequests.insert(
			entity,
			{TEXTURE_ASSET_ID::BULLET,
			 EFFECT_ASSET_ID::TEXTURED,
			 GEOMETRY_BUFFER_ID::SPRITE,
			RENDER_LAYER::MIDDLEGROUND});

	return entity;
}

Entity createEnemyProjectile(RenderSystem* render, vec2 position, float angle, Entity source)
{
	auto entity = Entity();

	Mesh& mesh = render->getMesh(GEOMETRY_BUFFER_ID::BULLET_CH);
	registry.meshPtrs.emplace(entity, &mesh);

	// Setting initial motion values
	Motion& motion = registry.motions.emplace(entity);
	Projectile& projectile = registry.projectiles.emplace(entity);
	motion.position = position;
	motion.look_angle = angle + M_PI / 4;
	motion.scale = vec2({ BULLET_BB_WIDTH, BULLET_BB_HEIGHT });
	motion.velocity = vec2({ 250.0f * cos(angle), 250.0f * sin(angle) });

	// Set the source of the projectile
	registry.projectiles.get(entity).source = source;

	// Set damage and projectile properties
	Deadly& deadly = registry.deadlies.emplace(entity);
	projectile.weapon_type = WeaponType::GATLING_GUN;
	projectile.lifetime = weapon_stats[projectile.weapon_type].lifetime * 100.f;
	deadly.damage = weapon_stats[projectile.weapon_type].damage;

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::BULLET,
		 EFFECT_ASSET_ID::TEXTURED,
		 GEOMETRY_BUFFER_ID::SPRITE,
		RENDER_LAYER::MIDDLEGROUND });

	return entity;
}

Entity createSniperProjectile(RenderSystem* render, vec2 position, float angle, float rng, float fire_length, Entity source)
{
	auto entity = Entity();

	// Actual firing angle is randomly perturbed based off the accuracy and how long the fire button has been held
	float accuracy = clamp(fire_length * 0.0005f, 0.0f, 0.4f);
	angle += (rng - 0.5f) * accuracy;

	// Setting initial motion values
	Motion& motion = registry.motions.emplace(entity);
	Projectile& projectile = registry.projectiles.emplace(entity);
	motion.position = position;
	motion.look_angle = angle + M_PI ;
	motion.scale = vec2({ 48.f, 48.f });
	motion.velocity = vec2({ 2000.0f * cos(angle), 2000.0f * sin(angle) });

	// Set the source of the projectile
	registry.projectiles.get(entity).source = source;

	// Set damage and projectile properties
	Deadly& deadly = registry.deadlies.emplace(entity);
	projectile.weapon_type = WeaponType::SNIPER;
	projectile.lifetime = weapon_stats[projectile.weapon_type].lifetime;
	deadly.damage = weapon_stats[projectile.weapon_type].damage;

	Animation& animation = registry.animations.emplace(entity);
	animation.sheet_id = SPRITE_SHEET_ID::BLUE_EFFECT;
	animation.total_frames = 4;
	animation.current_frame = 0;
	animation.sprites = { {11, 5}, {12, 5}, {13, 5}, {14, 5} };
	animation.frame_durations_ms = { 100, 100, 100, 100 };
	animation.loop = true;

	AnimationTimer& animation_timer = registry.animationTimers.emplace(entity);
	animation_timer.counter_ms = animation.frame_durations_ms[0];

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::TEXTURE_COUNT,
		 EFFECT_ASSET_ID::TEXTURED,
		 GEOMETRY_BUFFER_ID::SPRITE,
		RENDER_LAYER::MIDDLEGROUND});

	return entity;
}

Entity createEnemySniperProjectile(RenderSystem* render, vec2 position, float angle, Entity source)
{
	auto entity = Entity();

	// Setting initial motion values
	Motion& motion = registry.motions.emplace(entity);
	Projectile& projectile = registry.projectiles.emplace(entity);
	motion.position = position;
	motion.look_angle = angle + M_PI;
	motion.scale = vec2({ 48.f, 48.f });
	motion.velocity = vec2({ 500.0f * cos(angle), 500.0f * sin(angle) });

	// Set the source of the projectile
	registry.projectiles.get(entity).source = source;

	// Set damage and projectile properties
	Deadly& deadly = registry.deadlies.emplace(entity);
	projectile.weapon_type = WeaponType::SNIPER;
	projectile.lifetime = weapon_stats[projectile.weapon_type].lifetime * 4;
	deadly.damage = weapon_stats[projectile.weapon_type].damage;

	Animation& animation = registry.animations.emplace(entity);
	animation.sheet_id = SPRITE_SHEET_ID::GREEN_EFFECT;
	animation.total_frames = 4;
	animation.current_frame = 0;
	animation.sprites = { {11, 5}, {12, 5}, {13, 5}, {14, 5} };
	animation.frame_durations_ms = { 100, 100, 100, 100 };
	animation.loop = true;

	AnimationTimer& animation_timer = registry.animationTimers.emplace(entity);
	animation_timer.counter_ms = animation.frame_durations_ms[0];

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::TEXTURE_COUNT,
		 EFFECT_ASSET_ID::TEXTURED,
		 GEOMETRY_BUFFER_ID::SPRITE,
		RENDER_LAYER::MIDDLEGROUND });

	return entity;
}

Entity createShotgunProjectile(RenderSystem* render, vec2 position, float angle, float rng, float fire_length, int i, Entity source)
{
	auto entity = Entity();

	// Actual firing angle is randomly perturbed based on accuracy and how long the fire button has been held
	float accuracy = clamp(fire_length * 0.0005f, 0.0f, 0.4f);
	float perturbedAngle = angle + (rng - 0.5f) * accuracy;

	float coneWidth = 0.5f;

	// Calculate the angle for each shotgun projectile in a cone
	float coneAngle = perturbedAngle - coneWidth / 2 + i * coneWidth / 10;

	// Setting initial motion values
	Motion& motion = registry.motions.emplace(entity);
	Projectile& projectile = registry.projectiles.emplace(entity);
	motion.position = position;
	motion.look_angle = coneAngle;
	motion.scale = vec2({ BULLET_BB_WIDTH * 0.8, BULLET_BB_HEIGHT * 0.8 });
	motion.velocity = vec2({ 1000.0f * cos(coneAngle), 1000.0f * sin(coneAngle) });
	
	// Set the source of the projectile
	registry.projectiles.get(entity).source = source;

	// Set damage and projectile properties
	Deadly& deadly = registry.deadlies.emplace(entity);
	projectile.weapon_type = WeaponType::SHOTGUN;
	projectile.lifetime = weapon_stats[projectile.weapon_type].lifetime;
	deadly.damage = weapon_stats[projectile.weapon_type].damage;

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::BULLET,
		  EFFECT_ASSET_ID::TEXTURED,
		  GEOMETRY_BUFFER_ID::SPRITE,
		RENDER_LAYER::MIDDLEGROUND});

	return entity;
}

Entity createShotgunProjectile(RenderSystem* render, vec2 position, float angle, int i, Entity source)
{
	auto entity = Entity();

	// Setting initial motion values
	Motion& motion = registry.motions.emplace(entity);
	Projectile& projectile = registry.projectiles.emplace(entity);
	motion.position = position;
	motion.look_angle = angle;
	motion.scale = vec2({ BULLET_BB_WIDTH * 0.8, BULLET_BB_HEIGHT * 0.8 });
	motion.velocity = vec2({ 1000.0f * cos(angle), 1000.0f * sin(angle) });

	// Set the source of the projectile
	registry.projectiles.get(entity).source = source;

	// Set damage and projectile properties
	Deadly& deadly = registry.deadlies.emplace(entity);
	projectile.weapon_type = WeaponType::SHOTGUN;
	projectile.lifetime = weapon_stats[projectile.weapon_type].lifetime;
	deadly.damage = weapon_stats[projectile.weapon_type].damage;

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::BULLET,
		  EFFECT_ASSET_ID::TEXTURED,
		  GEOMETRY_BUFFER_ID::SPRITE,
		RENDER_LAYER::MIDDLEGROUND });

	return entity;
}

Entity createRocketProjectile(RenderSystem* render, vec2 position, float angle, float rng, float fire_length, Entity source)
{
	auto entity = Entity();

	// Actual firing angle is randomly perturbed based off the accuracy and how long the fire button has been held
	float accuracy = clamp(fire_length * 0.0005f, 0.0f, 0.4f);
	angle += (rng - 0.5f) * accuracy;

	Mesh& mesh = render->getMesh(GEOMETRY_BUFFER_ID::BULLET_CH);
	registry.meshPtrs.emplace(entity, &mesh);

	// Setting initial motion values
	Motion& motion = registry.motions.emplace(entity);
	Projectile& projectile = registry.projectiles.emplace(entity);
	motion.position = position;
	motion.look_angle = angle;
	motion.scale = vec2({ 64.f , 64.f });
	motion.velocity = vec2({ 400.0f * cos(angle), 400.0f * sin(angle) });

	Animation& animation = registry.animations.emplace(entity);
	animation.sheet_id = SPRITE_SHEET_ID::BLUE_EFFECT;
	animation.total_frames = 4;
	animation.current_frame = 0;
	animation.sprites = { {11, 1}, {12, 1}, {13, 1}, {14, 1} };
	animation.frame_durations_ms = { 100, 100, 100, 100 };
	animation.loop = true;

	AnimationTimer& animation_timer = registry.animationTimers.emplace(entity);
	animation_timer.counter_ms = animation.frame_durations_ms[0];
	
	// Set the source of the projectile
	registry.projectiles.get(entity).source = source;

	// Set damage and projectile properties
	Deadly& deadly = registry.deadlies.emplace(entity);
	projectile.weapon_type = WeaponType::ROCKET_LAUNCHER;
	projectile.lifetime = weapon_stats[projectile.weapon_type].lifetime;
	deadly.damage = weapon_stats[projectile.weapon_type].damage;

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::TEXTURE_COUNT,
		 EFFECT_ASSET_ID::TEXTURED,
		 GEOMETRY_BUFFER_ID::SPRITE,
		RENDER_LAYER::MIDDLEGROUND });

	return entity;
}

Entity createEnemyRocketProjectile(RenderSystem* render, vec2 position, float angle, Entity source)
{
	auto entity = Entity();

	// Setting initial motion values
	Motion& motion = registry.motions.emplace(entity);
	Projectile& projectile = registry.projectiles.emplace(entity);
	motion.position = position;
	motion.look_angle = angle;
	motion.scale = vec2({ 64.f, 64.f });
	motion.velocity = vec2({ 400.0f * cos(angle), 400.0f * sin(angle) });

	// Set the source of the projectile
	registry.projectiles.get(entity).source = source;

	// Set damage and projectile properties
	Deadly& deadly = registry.deadlies.emplace(entity);
	projectile.weapon_type = WeaponType::ROCKET_LAUNCHER;
	projectile.lifetime = weapon_stats[projectile.weapon_type].lifetime * 4;
	deadly.damage = weapon_stats[projectile.weapon_type].damage;

	Animation& animation = registry.animations.emplace(entity);
	animation.sheet_id = SPRITE_SHEET_ID::GREEN_EFFECT;
	animation.total_frames = 4;
	animation.current_frame = 0;
	animation.sprites = { {11, 1}, {12, 1}, {13, 1}, {14, 1} };
	animation.frame_durations_ms = { 100, 100, 100, 100 };
	animation.loop = true;

	AnimationTimer& animation_timer = registry.animationTimers.emplace(entity);
	animation_timer.counter_ms = animation.frame_durations_ms[0];

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::TEXTURE_COUNT,
				 EFFECT_ASSET_ID::TEXTURED,
				 GEOMETRY_BUFFER_ID::SPRITE,
				RENDER_LAYER::MIDDLEGROUND });

	return entity;
}

Entity createFlamethrowerProjectile(RenderSystem* render, vec2 position, float angle, float rng, float fire_length, Entity source)
{
	auto entity = Entity();

	// Actual firing angle is randomly perturbed based off the accuracy and how long the fire button has been held
	float accuracy = clamp(fire_length * 0.0005f, 0.0f, 0.4f);
	angle += (rng - 0.5f) * accuracy;

	Mesh& mesh = render->getMesh(GEOMETRY_BUFFER_ID::BULLET_CH);
	registry.meshPtrs.emplace(entity, &mesh);

	// Setting initial motion values
	Motion& motion = registry.motions.emplace(entity);
	Projectile& projectile = registry.projectiles.emplace(entity);
	motion.position = position;
	motion.look_angle = angle + M_PI / 4;
	motion.scale = vec2({ 32.f * glm::linearRand(0.8f, 1.2f), 32.f * glm::linearRand(0.8f, 1.2f) });
	motion.velocity = vec2({ 600.0f * cos(angle), 600.0f * sin(angle) });

	// Set the source of the projectile
	registry.projectiles.get(entity).source = source;

	// Set damage and projectile properties
	Deadly& deadly = registry.deadlies.emplace(entity);
	projectile.weapon_type = WeaponType::FLAMETHROWER;
	projectile.lifetime = weapon_stats[projectile.weapon_type].lifetime;
	deadly.damage = weapon_stats[projectile.weapon_type].damage;

	Animation& animation = registry.animations.emplace(entity);
	animation.sheet_id = SPRITE_SHEET_ID::RED_EFFECT;
	animation.total_frames = 4;
	animation.current_frame = 0;
	animation.sprites = { {6, 9}, {7, 9}, {8, 9}, {9, 9} };
	animation.frame_durations_ms = { 100, 100, 100, 100 };
	animation.loop = true;

	AnimationTimer& animation_timer = registry.animationTimers.emplace(entity);
	animation_timer.counter_ms = animation.frame_durations_ms[0];

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::TEXTURE_COUNT,
		 EFFECT_ASSET_ID::TEXTURED,
		 GEOMETRY_BUFFER_ID::SPRITE,
		RENDER_LAYER::MIDDLEGROUND });

	return entity;
}

Entity createEnergyHaloProjectile(RenderSystem* render, vec2 position, float angle, float rng, float fire_length, int i, Entity source)
{
	auto entity = Entity();

	float angle_increment = (2 * M_PI) / 16; // ADJUST BASED ON HOW MANY PROJECTILES
	float shoot_angle = angle_increment * i;

	Mesh& mesh = render->getMesh(GEOMETRY_BUFFER_ID::BULLET_CH);
	registry.meshPtrs.emplace(entity, &mesh);

	// Setting initial motion values
	Motion& motion = registry.motions.emplace(entity);
	Projectile& projectile = registry.projectiles.emplace(entity);
	motion.position = position;
	motion.look_angle = shoot_angle;
	motion.scale = vec2({ 64.f , 64.f });
	motion.velocity = vec2({ 0.f, 0.f });

	// Set the source of the projectile
	registry.projectiles.get(entity).source = source;

	// Set damage and projectile properties
	Deadly& deadly = registry.deadlies.emplace(entity);
	projectile.weapon_type = WeaponType::ENERGY_HALO;
	projectile.lifetime = weapon_stats[projectile.weapon_type].lifetime;
	deadly.damage = weapon_stats[projectile.weapon_type].damage;

	Animation& animation = registry.animations.emplace(entity);
	animation.sheet_id = SPRITE_SHEET_ID::BLUE_EFFECT;
	animation.total_frames = 4;
	animation.current_frame = 0;
	animation.sprites = { {11, 8}, {12, 8}, {13, 8}, {14, 8} };
	animation.frame_durations_ms = { 100, 100, 100, 100 };
	animation.loop = true;

	AnimationTimer& animation_timer = registry.animationTimers.emplace(entity);
	animation_timer.counter_ms = animation.frame_durations_ms[0];

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::TEXTURE_COUNT,
		 EFFECT_ASSET_ID::TEXTURED,
		 GEOMETRY_BUFFER_ID::SPRITE,
		RENDER_LAYER::MIDDLEGROUND });

	return entity;
}

Entity createEnemyFlamethrowerProjectile(RenderSystem* render, vec2 position, float angle, Entity source)
{
	auto entity = Entity();

	// Setting initial motion values
	Motion& motion = registry.motions.emplace(entity);
	Projectile& projectile = registry.projectiles.emplace(entity);
	motion.position = position;
	motion.look_angle = angle + M_PI / 4;
	motion.scale = vec2({ 32.f, 32.f });
	motion.velocity = vec2({ 600.0f * cos(angle), 600.0f * sin(angle) });

	// Set the source of the projectile
	registry.projectiles.get(entity).source = source;

	// Set damage and projectile properties
	Deadly& deadly = registry.deadlies.emplace(entity);
	projectile.weapon_type = WeaponType::FLAMETHROWER;
	projectile.lifetime = weapon_stats[projectile.weapon_type].lifetime * 4;
	deadly.damage = weapon_stats[projectile.weapon_type].damage;

	Animation& animation = registry.animations.emplace(entity);
	animation.sheet_id = SPRITE_SHEET_ID::RED_EFFECT;
	animation.total_frames = 4;
	animation.current_frame = 0;
	animation.sprites = { {6, 9}, {7, 9}, {8, 9}, {9, 9} };
	animation.frame_durations_ms = { 100, 100, 100, 100 };
	animation.loop = true;

	AnimationTimer& animation_timer = registry.animationTimers.emplace(entity);
	animation_timer.counter_ms = animation.frame_durations_ms[0];

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::TEXTURE_COUNT,
				 EFFECT_ASSET_ID::TEXTURED,
				 GEOMETRY_BUFFER_ID::SPRITE,
				RENDER_LAYER::MIDDLEGROUND });

	return entity;
}

Entity createStartScreen(RenderSystem* renderer) {
		auto entity = Entity();

	// Setting initial motion values
	Motion& motion = registry.motions.emplace(entity);
	motion.position = { window_width_px / 2, window_height_px / 2 };
	motion.scale = vec2({ window_width_px, window_height_px });

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::START_SCREEN,
				 EFFECT_ASSET_ID::TEXTURED,
				 GEOMETRY_BUFFER_ID::SPRITE,
				RENDER_LAYER::BACKGROUND });

	return entity;

}

Entity createDeathScreen(RenderSystem* renderer) {
	auto entity = Entity();

	// Setting initial motion values
	Motion& motion = registry.motions.emplace(entity);
	motion.position = { window_width_px / 2, window_height_px / 2 };
	motion.scale = vec2({ window_width_px, window_height_px });

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::DEATH_SCREEN,
				 EFFECT_ASSET_ID::TEXTURED,
				 GEOMETRY_BUFFER_ID::SPRITE,
				RENDER_LAYER::BACKGROUND });

	return entity;

}

// TODO: figure out whether invidiual components are smart and whether this should be moved to a separate file
void createWalls(RenderSystem* render, Room& room)
{
	auto topWall = Entity();
	auto bottomWall = Entity();
	auto leftWall = Entity();
	auto rightWall = Entity();

	float x_mid = window_width_px / 2;
	float y_mid = window_height_px / 2;
	float x_delta = game_window_size_px / 2 - 32;
	float y_delta = game_window_size_px / 2 - 32;
	float x_max = x_mid + x_delta;
	float x_min = x_mid - x_delta;
	float y_max = y_mid + y_delta;
	float y_min = y_mid - y_delta;

	// top wall
	Motion& top_motion = registry.motions.emplace(topWall);
	top_motion.position = vec2({ x_mid, y_min });
	top_motion.scale = vec2({ HORIZONTAL_WALL_BB_WIDTH, HORIZONTAL_WALL_BB_HEIGHT });

	registry.obstacles.emplace(topWall);
	registry.obstacles.get(topWall).is_wall = true;
	registry.renderRequests.insert(
		topWall,
		{ room.has_top_door ? TEXTURE_ASSET_ID::TOP_LEVEL1_FULL_WALL_OPEN_DOOR
			: TEXTURE_ASSET_ID::TOP_LEVEL1_FULL_WALL_CLOSED_DOOR,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE,
		RENDER_LAYER::MIDDLEGROUND});
	// create the door if room should have one
	if (room.has_top_door) {
		auto top_door = Entity();
		Motion& top_door_motion = registry.motions.emplace(top_door);
		top_door_motion.position = { x_mid, y_min };
		top_door_motion.scale = { 64,64 };
		Obstacle& top_door_obstacle = registry.obstacles.emplace(top_door);
		top_door_obstacle.is_passable = true;
		top_door_obstacle.is_top_door = true;
	}

	// bottom wall
	Motion& bottom_motion = registry.motions.emplace(bottomWall);
	bottom_motion.position = vec2({ x_mid, y_max });
	bottom_motion.scale = vec2({ HORIZONTAL_WALL_BB_WIDTH, HORIZONTAL_WALL_BB_HEIGHT });

	registry.obstacles.emplace(bottomWall);
	registry.renderRequests.insert(
		bottomWall,
		{ room.has_bottom_door ? TEXTURE_ASSET_ID::BOTTOM_LEVEL1_FULL_WALL_OPEN_DOOR
			: TEXTURE_ASSET_ID::BOTTOM_LEVEL1_FULL_WALL_CLOSED_DOOR,
					EFFECT_ASSET_ID::TEXTURED,
					GEOMETRY_BUFFER_ID::SPRITE,
		RENDER_LAYER::MIDDLEGROUND});
	registry.obstacles.get(bottomWall).is_wall = true;

	if (room.has_bottom_door) {
		auto bottom_door = Entity();
		Motion& bottom_door_motion = registry.motions.emplace(bottom_door);
		bottom_door_motion.position = { x_mid, y_max };
		bottom_door_motion.scale = { 64,64 };
		Obstacle& bottom_door_obstacle = registry.obstacles.emplace(bottom_door);
		bottom_door_obstacle.is_passable = true;
		bottom_door_obstacle.is_bottom_door = true;
	}
	// player can pass through the door if it exists
	// left wall
	Motion& left_motion = registry.motions.emplace(leftWall);
	left_motion.position = vec2({ x_min, y_mid });
	left_motion.scale = vec2({ VERTICAL_WALL_BB_HEIGHT , VERTICAL_WALL_BB_WIDTH });

	registry.obstacles.emplace(leftWall);
	registry.obstacles.get(leftWall).is_wall = true;

	registry.renderRequests.insert(
		leftWall,
		{ room.has_left_door ? TEXTURE_ASSET_ID::LEFT_LEVEL1_FULL_WALL_OPEN_DOOR
			: TEXTURE_ASSET_ID::LEFT_LEVEL1_FULL_WALL_CLOSED_DOOR ,
							EFFECT_ASSET_ID::TEXTURED,
							GEOMETRY_BUFFER_ID::SPRITE,
		RENDER_LAYER::MIDDLEGROUND });
	if (room.has_left_door) {
		auto left_door = Entity();
		Motion& left_door_motion = registry.motions.emplace(left_door);
		left_door_motion.position = { x_min, y_mid };
		left_door_motion.scale = { 64,64 };
		Obstacle& left_door_obstacle = registry.obstacles.emplace(left_door);
		left_door_obstacle.is_passable = true;
		left_door_obstacle.is_left_door = true;
	}

	// right wall
	Motion& right_motion = registry.motions.emplace(rightWall);
	right_motion.position = vec2({ x_max, y_mid });
	right_motion.scale = vec2({ VERTICAL_WALL_BB_HEIGHT, VERTICAL_WALL_BB_WIDTH });

	registry.obstacles.emplace(rightWall);
	registry.obstacles.get(rightWall).is_wall = true;

	registry.renderRequests.insert(
		rightWall,
		{ room.has_right_door ? TEXTURE_ASSET_ID::RIGHT_LEVEL1_FULL_WALL_OPEN_DOOR
			: TEXTURE_ASSET_ID::RIGHT_LEVEL1_FULL_WALL_CLOSED_DOOR ,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE,
		RENDER_LAYER::MIDDLEGROUND });

	if (room.has_right_door) {
		auto right_door = Entity();
		Motion& right_door_motion = registry.motions.emplace(right_door);
		right_door_motion.position = { x_max, y_mid };
		right_door_motion.scale = { 64,64 };
		Obstacle& right_door_obstacle = registry.obstacles.emplace(right_door);
		right_door_obstacle.is_passable = true;
		right_door_obstacle.is_right_door = true;
	}
}

void clearExistingWalls()
{
	for (Entity e : registry.motions.entities)
	{
		// remove all enemies, obstacles, animations
		if (registry.obstacles.has(e))
		{
			Obstacle obstacle = registry.obstacles.get(e);
			if (obstacle.is_wall)
			{
				registry.remove_all_components_of(e);
			}
		}
	}
}

void render_room(RenderSystem* render, Level& level)
{
	Room& current_room = registry.rooms.get(level.rooms[level.current_room]);
	
	if (!current_room.is_visited) {
		// set the room to visited
		current_room.is_visited = true;
		WorldGenerator world_generator;
		
		// After clearing each room, player has a higher chance of encountering a shop room.
		// Using the logistic function to determine whether to spawn a shop room or an enemy room.
		// Logistic function is defined as:
		//		
		//			f(x) = 1 / 1 + exp(-ax + b)
		//
		// - larger a -> larger increase of chance per room cleared
		// - larger b -> smaller increase of chance per room cleared
		std::knuth_b rnd_engine;
		std::bernoulli_distribution prob( 1 / (1 + exp(-0.5 * level.num_shop_spawn_counter + 2.f)) );

		if (prob(rnd_engine) && level.num_shop_spawned < 1000) {
			// Generate a shop room
			current_room.room_type = ROOM_TYPE::SHOP_ROOM;
			world_generator.generateNewRoom(current_room, level);
			
			Player& player = registry.players.components.back();
			std::vector<WeaponType> locked_weapons;
			
			for (int i = 0; i < (int)WeaponType::TOTAL_WEAPON_TYPES; i++) {
				// total_ammo_count of the smallest integer value implies locked weapons
				if (is_weapon_locked((WeaponType)i)) {
					locked_weapons.push_back((WeaponType)i);
				}
			}

			if (locked_weapons.size() > 0) {
				std::default_random_engine rng = std::default_random_engine(std::random_device()());
				std::uniform_int_distribution<int> weapon_idx_selector(0, locked_weapons.size() - 1);
				
				current_room.weapon_on_sale = locked_weapons[weapon_idx_selector(rng)];
			} else {
				current_room.weapon_on_sale = WeaponType::TOTAL_WEAPON_TYPES;
			}

			level.num_shop_spawned++;
			level.num_shop_spawn_counter = 0;

			std::cout << "shop room generated" << '\n';
		} else if (level.num_rooms_until_boss <= 0) {
			// Generate a boss room
			current_room.room_type = ROOM_TYPE::BOSS_ROOM;
			world_generator.generateNewRoom(current_room, level);

			std::cout << "boss room generated, back to rendering" << std::endl;
			
			level.num_rooms_until_boss = NUM_ROOMS_UNTIL_BOSS;
		} else {
			// Generate a normal room
			current_room.room_type = ROOM_TYPE::NORMAL_ROOM;
			world_generator.generateNewRoom(current_room, level);
		}
	} else {
		std::cout << "revisiting room!" << std::endl;
	}

	// in case current room was not visited, re-retrieve current room 
	Room& room_to_render = registry.rooms.get(level.rooms[level.current_room]);

	float x_origin = (window_width_px / 2) - (game_window_size_px / 2) + 32;
	float y_origin = (window_height_px / 2) - (game_window_size_px / 2) + 32;

	for (auto& pos : room_to_render.obstacle_positions)
	{
		float x = x_origin + pos.x * game_window_block_size;
		float y = y_origin + pos.y * game_window_block_size;
		createObstacle(render, vec2(x, y));
	}

	// Specify types for each enemy, later need to find a way to assign types randomly now its 2 ranged 1 melee
	std::vector<AI::AIType> enemy_types = { AI::AIType::MELEE, AI::AIType::RANGED, AI::AIType::TURRET, AI::AIType::SHOTGUN, AI::AIType::ROCKET, AI::AIType::FLAMETHROWER };

	// Create each enemy with their specified type
	for (auto& pos : room_to_render.enemy_positions) {
		//enemy positions is a set of vec2
		float x = x_origin + pos.x * game_window_block_size;
		float y = y_origin + pos.y * game_window_block_size;
		createEnemy(render, vec2(x, y), 500.0f, enemy_types[rand() % enemy_types.size()]);
	}

	// Create a shop panel if a shop room is encountered
	if (room_to_render.room_type == ROOM_TYPE::SHOP_ROOM) {
		createShopPanel(render, current_room.weapon_on_sale);
	}

	createWalls(render, room_to_render);
}

Entity createShopPanel(RenderSystem* renderer, WeaponType weapon_on_sale) {
	auto entity = Entity();

	Motion& motion = registry.motions.emplace(entity);
	motion.position = { window_width_px / 2, window_height_px / 2 };
	motion.scale = { game_window_block_size, game_window_block_size };

	ShopPanel& shop = registry.shopPanels.emplace(entity);
	shop.weapon_on_sale = weapon_on_sale;

	return entity;
}

Entity createShopIndicator(RenderSystem* renderer, vec2 position) {
	Entity text_e = createText(renderer, "E", position, 0.75f, { 1.f, 1.f, 1.f }, TextAlignment::CENTER);
	registry.renderRequests.emplace(text_e).used_render_layer = RENDER_LAYER::FOREGROUND;
	registry.debugComponents.emplace(text_e);

	return text_e;
}

Entity createLine(vec2 position, vec2 scale, float angle, vec3 color)
{
	Entity entity = Entity();

	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::TEXTURE_COUNT,
		 EFFECT_ASSET_ID::LINE,
		 GEOMETRY_BUFFER_ID::DEBUG_LINE,
		RENDER_LAYER::FOREGROUND });

	// Create motion
	Motion& motion = registry.motions.emplace(entity);
	motion.velocity = { 0, 0 };
	motion.position = position;
	motion.look_angle = angle;
	motion.scale = scale;

	registry.colors.emplace(entity) = {1.f, 0.f, 0.f};

	registry.noCollisionChecks.emplace(entity);
	registry.debugComponents.emplace(entity);
	return entity;
}

vec2 getTextRectSize(RenderSystem* renderer, std::string& text, float font_size_scale) {
	std::map<char, Character> ftChars = renderer->m_ftCharacters;

	float str_w = 0.f;
	float str_h = 0.f;

	for (int i = 0; i < text.length(); i++) {
		const Character ch = ftChars[text[i]];
		str_w += (ch.Advance >> 6) * font_size_scale;
		str_h = max(str_h, ch.Size.y * font_size_scale);
	}

	return vec2(str_w, str_h);
}

Entity createText(RenderSystem* renderer, std::string content, vec2 pos, float scale, vec3 color, TextAlignment alignment)
{
	auto entity = Entity();

	Text& text = registry.texts.emplace(entity);
	text.content = content;
	text.color = color;
	text.alignment = alignment;

	text.rect_size = getTextRectSize(renderer, content, scale);

	Motion& motion = registry.motions.emplace(entity);
	motion.position = { pos.x, window_height_px - pos.y }; // flip y axis as text is rendered from top to bottom, but we use bottom to top everywhere else
	motion.scale = vec2({ scale, scale });

	return entity;
}

Entity createStatusHud(RenderSystem* render)
{
	auto entity = Entity();

	// Setting initial motion values
	Motion& motion = registry.motions.emplace(entity);
	motion.position = vec2{ window_width_px / 2, window_height_px / 2 };
	motion.scale = vec2({ window_width_px, window_height_px });

	registry.noCollisionChecks.emplace(entity);

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::PLAYER_STATUS_HUD,
				 EFFECT_ASSET_ID::TEXTURED,
				 GEOMETRY_BUFFER_ID::SPRITE,
				RENDER_LAYER::FOREGROUND });

	return entity;
}

Entity createMuzzleFlash(RenderSystem* render, Entity source) {
	auto entity = Entity();

	// Setting initial motion values
	Motion& source_motion = registry.motions.get(source);
	Motion& motion = registry.motions.emplace(entity);
	motion.position = source_motion.position + (52.f * vec2{ cos(source_motion.look_angle - M_PI/2), sin(source_motion.look_angle - M_PI/2) });
	motion.scale = vec2({ 48.f, 48.f });
	motion.look_angle = source_motion.look_angle - M_PI;

	Animation& animation = registry.animations.emplace(entity);
	animation.sheet_id = SPRITE_SHEET_ID::BLUE_EFFECT;	
	animation.total_frames = 4;
	animation.current_frame = 0;
	animation.sprites = { {11, 13}, {12, 13}, {13, 13}, {14, 13} };
	animation.frame_durations_ms = { 50, 50, 50, 50 };
	animation.loop = false;

	MuzzleFlashTimer& muzzle_flash_timer = registry.muzzleFlashTimers.emplace(entity);
	muzzle_flash_timer.counter_ms = 400;
	muzzle_flash_timer.source = source;

	AnimationTimer& animation_timer = registry.animationTimers.emplace(entity);
	animation_timer.counter_ms = animation.frame_durations_ms[0];

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::TEXTURE_COUNT,
		 EFFECT_ASSET_ID::TEXTURED,
		 GEOMETRY_BUFFER_ID::SPRITE,
		RENDER_LAYER::UI });

	return entity;
}

Entity createExplosion(RenderSystem* render, vec2 pos, float scale, bool repeat)
{
	auto entity = Entity();

	// Setting initial motion values
	Motion& motion = registry.motions.emplace(entity);
	motion.position = pos;
	motion.scale = vec2({ EXPLOSION_BB_WIDTH * scale, EXPLOSION_BB_HEIGHT * scale });

	assert(!registry.animations.has(entity));
	Animation& animation = registry.animations.emplace(entity);
	animation.sheet_id = SPRITE_SHEET_ID::EXPLOSION;
	animation.total_frames = 12;
	animation.current_frame = 0;
	animation.sprites = { {0, 0}, {1, 0}, {2, 0}, {3, 0}, {4, 0}, {5, 0}, {6, 0}, {7, 0}, {8, 0}, {9, 0}, {10, 0}, {11, 0} };
	animation.frame_durations_ms = { 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100 };
	animation.loop = repeat;

	assert(!registry.animationTimers.has(entity));
	AnimationTimer& animation_timer = registry.animationTimers.emplace(entity);
	animation_timer.counter_ms = animation.frame_durations_ms[0];

	registry.renderRequests.insert(entity, {
		TEXTURE_ASSET_ID::TEXTURE_COUNT,
		EFFECT_ASSET_ID::TEXTURED,
		GEOMETRY_BUFFER_ID::SPRITE,
		RENDER_LAYER::FOREGROUND});

	return entity;
}

Entity createFire(RenderSystem* render, vec2 pos, float scale, bool repeat)
{
	auto entity = Entity();

	// Setting initial motion values
	Motion& motion = registry.motions.emplace(entity);
	motion.position = pos;
	motion.scale = vec2({ FIRE_BB_WIDTH * scale, FIRE_BB_HEIGHT * scale });

	assert(!registry.animations.has(entity));
	Animation& animation = registry.animations.emplace(entity);
	animation.sheet_id = SPRITE_SHEET_ID::RED_EFFECT;
	animation.total_frames = 4;
	animation.current_frame = 0;
	animation.sprites = { {11, 7}, {12, 7}, {13, 7}, {14, 7} };
	animation.frame_durations_ms = { 100, 100, 100, 100 };
	animation.loop = repeat;

	assert(!registry.animationTimers.has(entity));
	AnimationTimer& animation_timer = registry.animationTimers.emplace(entity);
	animation_timer.counter_ms = animation.frame_durations_ms[0];

	registry.renderRequests.insert(entity, {
		TEXTURE_ASSET_ID::TEXTURE_COUNT,
		EFFECT_ASSET_ID::TEXTURED,
		GEOMETRY_BUFFER_ID::SPRITE,
		RENDER_LAYER::FOREGROUND });

	return entity;
}

Entity createBulletImpact(RenderSystem* render, vec2 pos, float scale, bool repeat)
{
	auto entity = Entity();

	// Setting initial motion values
	Motion& motion = registry.motions.emplace(entity);
	motion.position = pos;
	motion.scale = vec2({ BULLET_IMPACT_BB_WIDTH * scale, BULLET_IMPACT_BB_HEIGHT * scale });

	assert(!registry.animations.has(entity));
	Animation& animation = registry.animations.emplace(entity);
	animation.sheet_id = SPRITE_SHEET_ID::YELLOW_EFFECT;
	animation.total_frames = 4;
	animation.current_frame = 0;
	animation.sprites = { {6, 7}, {7, 7}, {8, 7}, {9, 7} };
	animation.frame_durations_ms = { 50, 50, 50, 50 };
	animation.loop = repeat;

	assert(!registry.animationTimers.has(entity));
	AnimationTimer& animation_timer = registry.animationTimers.emplace(entity);
	animation_timer.counter_ms = animation.frame_durations_ms[0];

	registry.renderRequests.insert(entity, {
		TEXTURE_ASSET_ID::TEXTURE_COUNT,
		EFFECT_ASSET_ID::TEXTURED,
		GEOMETRY_BUFFER_ID::SPRITE,
		RENDER_LAYER::FOREGROUND });

	return entity;
}

Entity createWeaponEquippedIcon(RenderSystem* render, vec2 pos, TEXTURE_ASSET_ID textureId)
{
	auto entity = Entity();

	// Setting initial motion values
	Motion& motion = registry.motions.emplace(entity);
	motion.position = pos;
	motion.scale = vec2({ WEAPON_EQUIPPED_ICON_BB_WIDTH, WEAPON_EQUIPPED_ICON_BB_HEIGHT });

	registry.renderRequests.insert(
		entity,
		{ textureId,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE,
		RENDER_LAYER::UI });

	return entity;
}

Entity createWeaponUnequippedIcon(RenderSystem* render, vec2 pos, TEXTURE_ASSET_ID textureId)
{
	auto entity = Entity();

	// Setting initial motion values
	Motion& motion = registry.motions.emplace(entity);
	motion.position = pos;
	motion.scale = vec2({ WEAPON_UNEQUIPPED_ICON_BB_WIDTH, WEAPON_UNEQUIPPED_ICON_BB_HEIGHT });

	registry.renderRequests.insert(
		entity,
		{ textureId,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE,
		RENDER_LAYER::UI });
	
	return entity;
}

Entity createIconInfinity(RenderSystem* render, vec2 pos)
{
	auto entity = Entity();

	// Setting initial motion values
	Motion& motion = registry.motions.emplace(entity);
	motion.position = pos;
	motion.scale = vec2({ ICON_INFINITY_BB_WIDTH, ICON_INFINITY_BB_HEIGHT });

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::INFINITY_AMMO,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE,
		RENDER_LAYER::UI });

	return entity;
}

Entity createLevel(RenderSystem* render)
{
	auto entity = Entity();

	Level& level = registry.levels.emplace(entity);
	level.current_level = 1;

	// create a starting room with a room on each side
	auto starting_room_entity = Entity();
	level.current_room = std::pair<int, int>(0, 0);
	level.rooms.emplace(level.current_room, starting_room_entity);

	Room& starting_room = registry.rooms.emplace(starting_room_entity);
	WorldGenerator world_generator;

	// modifies Room component using pointer to Room component
	world_generator.generateStartingRoom(starting_room, level);

	render_room(render, level);
	return entity;
}
