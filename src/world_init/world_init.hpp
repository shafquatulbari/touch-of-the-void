#pragma once

#include "common/common.hpp"
#include "ecs/ecs.hpp"
#include "render_system/render_system.hpp"

// These are hard coded to the dimensions of the entity texture
// player is 32x32px and is standard for most
const float PLAYER_BB_WIDTH = 32.0f;
const float PLAYER_BB_HEIGHT = 32.0f;

const float OBSTACLE_BB_WIDTH = 32.0f;
const float OBSTACLE_BB_HEIGHT = 32.0f;

const float BACKGROUND_BB_WIDTH = 480.0f;
const float BACKGROUND_BB_HEIGHT = 480.0f;

const float BULLET_BB_WIDTH = 8.0f;
const float BULLET_BB_HEIGHT = 8.0f;

// the player
Entity createPlayer(RenderSystem* renderer, vec2 pos);
// the prey
Entity createEnemy(RenderSystem* renderer, vec2 position, float health_points);
// the obstacle
Entity createObstacle(RenderSystem* renderer, vec2 position, float health_points);
// the enemy
Entity createBullet(RenderSystem* renderer, vec2 position);
// a red line for debugging purposes
Entity createLine(vec2 position, vec2 size);
// the background
Entity createBackground(RenderSystem* renderer, vec2 position);
// a single projectile
Entity createProjectile(RenderSystem* renderer, vec2 position, float angle);

