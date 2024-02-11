#pragma once

#include <vector>

#include "ecs_registry/ecs_registry.hpp"
#include "common/common.hpp"

class AISystem
{
public:
	void step(float elapsed_ms);
};