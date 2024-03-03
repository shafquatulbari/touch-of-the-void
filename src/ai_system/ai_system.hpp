#pragma once

#include <vector>

#include "ecs_registry/ecs_registry.hpp"
#include "common/common.hpp"

class AISystem
{
public:
	void step(float elapsed_ms);
	void idleState(Entity entity, Motion& motion);
	void activeState(Entity entity, Motion& motion, float elapsed_ms);
	void handleRangedAI(Entity entity, Motion& motion, AI& ai, float elapsed_ms);
	bool lineOfSightClear(const vec2& start, const vec2& end, float fraction);
	void createProjectileForEnemy(vec2 position, float angle, Entity source);
	vec2 worldToGrid(const vec2& pos);
	vec2 gridToWorld(const vec2& gridPos);
	void updateGrid();
	std::vector<vec2> findPath(const vec2& startWorld, const vec2& goalWorld);
};