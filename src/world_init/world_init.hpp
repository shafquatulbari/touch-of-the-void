#pragma once

#include "common/common.hpp"
#include "ecs/ecs.hpp"
#include "render_system/render_system.hpp"

// These are hard coded to the dimensions of the entity texture
// player is 32x32px and is standard for most
const float PLAYER_BB_WIDTH = 64.0f;
const float PLAYER_BB_HEIGHT = 64.0f;

const float ENEMY_BB_WIDTH = 64.0f;
const float ENEMY_BB_HEIGHT = 64.0f;

const float OBSTACLE_BB_WIDTH = 64.0f;
const float OBSTACLE_BB_HEIGHT = 64.0f;

const float BACKGROUND_BB_WIDTH = 960.f;
const float BACKGROUND_BB_HEIGHT = 960.f;

const float BULLET_BB_WIDTH = 16.0f;
const float BULLET_BB_HEIGHT = 16.0f;

const float EXPLOSION_BB_WIDTH = 192.0f;
const float EXPLOSION_BB_HEIGHT = 192.0f;

const float WALL_BB_WIDTH = 832.0f;
const float WALL_BB_HEIGHT = 64.0F;

// the player
Entity createPlayer(RenderSystem* renderer, vec2 pos);
// the enemy
Entity createEnemy(RenderSystem* renderer, vec2 position, float health_points, AI::AIType aiType);
// the obstacle
Entity createObstacle(RenderSystem* renderer, vec2 position);
// the background
Entity createBackground(RenderSystem* renderer);
// a single projectile
Entity createProjectile(RenderSystem* renderer, vec2 position, float angle, float rng, float fire_length, Entity source);
// a single sinper projectile
Entity createSniperProjectile(RenderSystem* renderer, vec2 position, float angle, float rng, float fire_length, Entity source);
// a single shotgun projectile
Entity createShotgunProjectile(RenderSystem* renderer, vec2 position, float angle, float rng, float fire_length, int i, Entity source);
// debug line
Entity createLine(vec2 position, vec2 scale);
// a hard coded room
Entity createRoom(RenderSystem* renderer);
// a text object
Entity createText(RenderSystem* renderer, std::string content, vec2 pos, float scale, vec3 color);
// render the room
void render_room(RenderSystem* renderer, Room& room);
// an explosion animation
Entity createExplosion(RenderSystem* renderer, vec2 position, bool repeat);
