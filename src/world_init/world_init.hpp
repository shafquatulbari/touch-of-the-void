#pragma once

#include "common/common.hpp"
#include "ecs/ecs.hpp"
#include "render_system/render_system.hpp"

// These are hard coded to the dimensions of the entity texture
// player is 32x32px and is standard for most
const float PLAYER_BB_WIDTH = 32.0f;
const float PLAYER_BB_HEIGHT = 32.0f;

// the player
Entity createPlayer(RenderSystem* renderer, vec2 pos);
// the prey
Entity createEnemy(RenderSystem* renderer, vec2 position);
// the powerup
Entity createObstacle(RenderSystem* renderer, vec2 position);
// the enemy
Entity createBullet(RenderSystem* renderer, vec2 position);
// a red line for debugging purposes
Entity createLine(vec2 position, vec2 size);

