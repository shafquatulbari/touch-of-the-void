#include "boss.hpp"
#include "ai_system/ai_system.hpp"
#include <world_system/world_system.hpp>
#include <world_init/world_init.hpp>
#include <components/components.hpp>
#include <queue> // For priority queue (open list)
#include <unordered_set>  
#include <algorithm> 
#include <vector>
#include <cmath>
#include <memory>
#include <map>
#include <iostream>

void Boss::step(float elapsed_ms)
{
}

void Boss::handleDefensiveState(Entity entity, BossAI& boss, Motion& motion, float elapsed_ms)
{
}

void Boss::handleOffensiveState(Entity entity, BossAI& boss, Motion& motion, float elapsed_ms)
{
	
}