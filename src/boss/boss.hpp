#pragma once

#include <vector>

#include "render_system/render_system.hpp"
#include "ecs_registry/ecs_registry.hpp"
#include "common/common.hpp"

class Boss
{
private:
	RenderSystem* renderer;
public:
	Boss(RenderSystem* _renderer) : renderer(_renderer) {};

	void step(float elapsed_ms);

	void handleDefensiveState(Entity entity, BossAI& boss, Motion& motion, float elapsed_ms);

	void handleOffensiveState(Entity entity, BossAI& boss, Motion& motion, float elapsed_ms);

};