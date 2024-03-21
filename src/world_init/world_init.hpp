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

const float FIRE_BB_WIDTH = 32.0f;
const float FIRE_BB_HEIGHT = 32.0f;

const float BULLET_IMPACT_BB_WIDTH = 32.0f;
const float BULLET_IMPACT_BB_HEIGHT = 32.0f;

const float WALL_BB_WIDTH = 832.0f;
const float WALL_BB_HEIGHT = 64.0F;

const float WEAPON_EQUIPPED_ICON_BB_WIDTH = 192.0f;
const float WEAPON_EQUIPPED_ICON_BB_HEIGHT = 192.0f;

const float WEAPON_UNEQUIPPED_ICON_BB_WIDTH = 128.0f;
const float WEAPON_UNEQUIPPED_ICON_BB_HEIGHT = 128.0f;

const float ICON_INFINITY_BB_WIDTH = 122.0f;
const float ICON_INFINITY_BB_HEIGHT = 54.0f;

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
// a single rocket projectile
Entity createRocketProjectile(RenderSystem* renderer, vec2 position, float angle, float rng, float fire_length, Entity source);
// a single flamethrower projectile
Entity createFlamethrowerProjectile(RenderSystem* renderer, vec2 position, float angle, float rng, float fire_length, Entity source);
// debug line
Entity createLine(vec2 position, vec2 scale);
// a hard coded room
Entity createRoom(RenderSystem* renderer);
// a text object
Entity createText(RenderSystem* renderer, std::string content, vec2 pos, float scale, vec3 color, TextAlignment alignment);
// render the room
//void render_room(RenderSystem* renderer, Room& room);
void render_room(RenderSystem* renderer, Level& level);

// an explosion animation
Entity createExplosion(RenderSystem* renderer, vec2 position, float scale, bool repeat);
// a fire animation
Entity createFire(RenderSystem* renderer, vec2 position, float scale, bool repeat);
// a bullet impact animation
Entity createBulletImpact(RenderSystem* renderer, vec2 position, float scale, bool repeat);
// a player status HUD
Entity createStatusHud(RenderSystem* render);
// a weapon equipped icon
Entity createWeaponEquippedIcon(RenderSystem* render, vec2 pos, TEXTURE_ASSET_ID textureId);
// a weapon unequipped icon
Entity createWeaponUnequippedIcon(RenderSystem* render, vec2 pos, TEXTURE_ASSET_ID textureId);
// an infinity icon
Entity createIconInfinity(RenderSystem* render, vec2 pos);
// a game level
Entity createLevel(RenderSystem* render);
