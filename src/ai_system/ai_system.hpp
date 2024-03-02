#pragma once

#include <vector>

#include "ecs_registry/ecs_registry.hpp"
#include "common/common.hpp"

class AISystem
{
private:
	//Entity player = registry.players.entities[0];
	const int window_width_px = 1024;
	const int window_height_px = 512;
	const int game_window_size_px = 480;
	const int game_window_block_size = 32;
public:
	void step(float elapsed_ms);
	void idleState(Entity entity, Motion& motion);
	void activeState(Entity entity, Motion& motion, float elapsed_ms);
	void handleRangedAI(Entity entity, Motion& motion, AI& ai, float elapsed_ms);
	bool lineOfSightClear(const vec2& start, const vec2& end);
	void createProjectileForEnemy(vec2 position, float angle);
	vec2 worldToGrid(const vec2& pos);
	vec2 gridToWorld(const vec2& gridPos);
	void updateGrid();
	std::vector<vec2> findPath(const vec2& startWorld, const vec2& goalWorld);
};