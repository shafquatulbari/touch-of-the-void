#pragma once

#include <vector>

#include "render_system/render_system.hpp"
#include "ecs_registry/ecs_registry.hpp"
#include "common/common.hpp"

class AISystem
{
private:
	RenderSystem* renderer;
public:
	AISystem(RenderSystem* _renderer) : renderer(_renderer) {};

	bool AISystem::lineOfSightClear(const vec2& start, const vec2& end);

	bool isObstacleAtPosition(const vec2& gridPosition);

	std::vector<vec2> generateNeighbors(const vec2& position);

	std::vector<vec2> findPathAStar(const vec2& start, const vec2& goal);

	vec2 flockMovement(Entity entity, Motion& motion, float elapsed_ms, const vec2& playerPosition, float playerAvoidanceDistance);
	
	void step(float elapsed_ms);

	void idleState(Entity entity, AI& ai, Motion& motion, float elapsed_ms);

	void activeState(Entity entity, Motion& motion, float elapsed_ms);

	void handleRangedAI(Entity entity, Motion& motion, AI& ai, float elapsed_ms, const vec2& playerPosition);

	void handleMeleeAI(Entity entity, Motion& motion, AI& ai, float elapsed_ms, const vec2& playerPosition);

	void createProjectileForEnemy(vec2 position, float angle, Entity source);

	bool isPositionWithinBounds(const vec2& position);

	vec2 limit(vec2 v, float max);

	void handleTurretAI(Entity entity, Motion& motion, AI& ai, float elapsed_ms, const vec2& playerPosition);

	void handleShotgunAI(Entity entity, Motion& motion, AI& ai, float elapsed_ms, const vec2& playerPosition);

	vec2 clampPositionToBounds(const vec2& position);
};