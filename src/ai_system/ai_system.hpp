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
	AISystem() {};
	AISystem(RenderSystem* _renderer): renderer(_renderer) {}

	bool lineOfSightClear(const vec2& start, const vec2& end);

	bool isObstacleAtPosition(const vec2& position);

	std::vector<vec2> generateNeighbors(const vec2& position);

	std::vector<vec2> findPathAStar(const vec2& start, const vec2& goal);

	void step(float elapsed_ms);
	void idleState(Entity entity, Motion& motion);
	void activeState(Entity entity, Motion& motion, float elapsed_ms);
	void handleRangedAI(Entity entity, Motion& motion, AI& ai, float elapsed_ms, const vec2& playerPosition);
	bool lineOfSightClear(const vec2& start, const vec2& end, float fraction);
	void createProjectileForEnemy(vec2 position, float angle, Entity source);

};