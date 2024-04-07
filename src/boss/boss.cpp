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
#include <glm/trigonometric.hpp>

void Boss::step(float elapsed_ms)
{
    auto& bossesRegistry = registry.bosses;
    for (uint i = 0; i < bossesRegistry.size(); i++)
    {
        BossAI& boss = bossesRegistry.components[i];
        Entity entity = bossesRegistry.entities[i];
        Motion& motion = registry.motions.get(entity);
        auto& renderComponent = registry.renderRequests.get(entity); // Assuming this gets the rendering component

        // Check AI state and perform actions accordingly
        switch (boss.state) {
            case BossAI::BossState::DEFENSIVE:
				handleDefensiveState(entity, boss, motion, elapsed_ms);
				break;
            case BossAI::BossState::OFFENSIVE:
                handleOffensiveState(entity, boss, motion, elapsed_ms);
                break;
            case BossAI::BossState::GUIDED_MISSILE:
                handleGuidedMissile(entity, boss, motion, elapsed_ms);
                break;
        default:
            printf("Unknown state\n");
            break;
        }
        // Always update position based on velocity, regardless of state
        motion.position += motion.velocity * elapsed_ms / 1000.0f;
    }	
}

void Boss::handleDefensiveState(Entity entity, BossAI& boss, Motion& motion, float elapsed_ms)
{
    /*
    Animation& animation = registry.animations.get(entity);
    AnimationTimer& animationTimer = registry.animationTimers.get(entity);
    animationTimer.counter_ms += elapsed_ms;
    animation.sheet_id = SPRITE_SHEET_ID::ENEMY_BOSS_SHIELD;
    animation.total_frames = 19;
    animation.sprites = { {0, 0}, {1, 0}, {2, 0}, {3, 0}, {4, 0}, {5, 0}, {6, 0}, {7, 0}, {8, 0}, {9, 0}, {10, 0}, {11, 0}, {12, 0}, {13, 0}, {14, 0}, {15, 0}, {16, 0}, {17, 0}, {18, 0} };
    animation.frame_durations_ms = { 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100 };
    animation.loop = true;
    animation.current_frame = 0;
    */
    // Update the shoot timer
    boss.shootTimer += elapsed_ms / 1000.0f; // Convert milliseconds to seconds

    if (boss.shootTimer >= boss.shootCooldown) {
        boss.shootTimer = 0.0f; // Reset the timer

        // Determine the angle to the player
        auto playerPosition = registry.motions.get(registry.players.entities[0]).position;
        vec2 direction = normalize(playerPosition - motion.position);
        float shootingAngle = atan2(direction.y, direction.x);

        // Widen the spray angle and adjust the number of projectiles if needed
        int num_projectiles = 5;
        float spread_angle_degrees = 90.0f; // Wider angle in degrees

        // Calculate the starting angle for the spread
        float angle_step = glm::radians(spread_angle_degrees) / static_cast<float>(num_projectiles - 1);
        float start_angle = shootingAngle - glm::radians(spread_angle_degrees) / 2.0f;

        // Shoot projectiles in a spread
        for (int i = 0; i < num_projectiles; i++) {
            float angle = start_angle + angle_step * i;
            createBossProjectile(renderer, motion.position, angle, 0.0, 0.0, i, entity);
        }
    }
    // Update the state timer
    boss.stateTimer += elapsed_ms / 1000.0f; // Convert milliseconds to seconds

    if (boss.stateTimer >= boss.stateDuration) {
        boss.state = BossAI::BossState::OFFENSIVE; // Switch to offensive state
        // Reset timers and counts for the next state transition
        boss.stateTimer = 0.0f;
        boss.enemyCreationTimer = 0.0f;
        boss.aliveEnemyCount = 0; // Make sure to reset this count when switching to Offensive
        if (registry.healths.get(entity).current_health <= (registry.healths.get(entity).max_health * 0.4f)) {
            boss.state = BossAI::BossState::GUIDED_MISSILE;
        }
    }
}

void Boss::handleOffensiveState(Entity entity, BossAI& boss, Motion& motion, float elapsed_ms)
{
    /*
    Animation& animation = registry.animations.get(entity);
    AnimationTimer& animationTimer = registry.animationTimers.get(entity);
    animationTimer.counter_ms += elapsed_ms;
    animation.sheet_id = SPRITE_SHEET_ID::ENEMY_BOSS_SPAWN;
    animation.total_frames = 38;
    animation.sprites = { {0, 0}, {1, 0}, {2, 0}, {3, 0}, {4, 0}, {5, 0}, {6, 0}, {7, 0}, {8, 0}, {9, 0}, {10, 0}, {11, 0}, {12, 0}, {13, 0}, {14, 0}, {15, 0}, {16, 0}, {17, 0}, {18, 0}, {19, 0}, {20, 0}, {21, 0}, {22, 0}, {23, 0}, {24, 0}, {25, 0}, {26, 0}, {27, 0}, {28, 0}, {29, 0}, {30, 0}, {31, 0}, {32, 0}, {33, 0}, {34, 0}, {35, 0}, {36, 0}, {37, 0} };
    animation.frame_durations_ms = { 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50 };
    animation.loop = true;
    animation.current_frame = 0;
    */
    // Specify types for each enemy, later need to find a way to assign types randomly now its 2 ranged 1 melee
    std::vector<AI::AIType> enemy_types = { AI::AIType::MELEE };
    //enemy positions is a set of vec2
    vec2 pos = { 7.0f, 3.0f };
    float x_origin = (window_width_px / 2) - (game_window_size_px / 2) + 32;
    float y_origin = (window_height_px / 2) - (game_window_size_px / 2) + 32;
    float x = x_origin + pos.x * game_window_block_size;
    float y = y_origin + pos.y * game_window_block_size;
    // Adjust state duration for offensive state
    boss.stateDuration = 10.0f;
    boss.enemyCreationTimer += elapsed_ms / 1000.0f;
    boss.stateTimer += elapsed_ms / 1000.0f; // Convert milliseconds to seconds

    // Check if it's time to switch state
    if (boss.stateTimer >= boss.stateDuration && boss.aliveEnemyCount == 0) {
        // if boss is at 80% health, switch to another state
        if (registry.healths.get(entity).current_health <= (registry.healths.get(entity).max_health * 0.4f)) {
			boss.state = BossAI::BossState::GUIDED_MISSILE;
		}
        boss.state = BossAI::BossState::DEFENSIVE; // Switch to defensive state only if all enemies are dead
        // Reset timers and counts for the next state transition
        boss.stateTimer = 0.0f;
        boss.enemyCreationTimer = 0.0f;
        boss.totalSpawnedEnemies = 0;
    }
   
    int maxEnemies = 5; // Maximum number of enemies that can be alive at once
    auto enemyType = enemy_types[rand() % enemy_types.size()]; // Random type from predefined vector
    if (boss.enemyCreationTimer >= boss.enemyCreationCooldown && boss.totalSpawnedEnemies < maxEnemies) {
        boss.enemyCreationTimer = 0.0f; // Reset timer for next enemy creation
        createEnemy(renderer, vec2(x, y), 500.0f, enemyType, true);
        boss.aliveEnemyCount++;
        boss.totalSpawnedEnemies++;
    }
}

void Boss::handleGuidedMissile(Entity entity, BossAI& boss, Motion& motion, float elapsed_ms) {
    /*
    Animation& animation = registry.animations.get(entity);
    animation.sheet_id = SPRITE_SHEET_ID::ENEMY_BOSS_IDLE;
    animation.total_frames = 19;
    animation.sprites = { {0, 0}, {1, 0}, {2, 0}, {3, 0}, {4, 0}, {5, 0}, {6, 0}, {7, 0}, {8, 0}, {9, 0}, {10, 0}, {11, 0}, {12, 0}, {13, 0}, {14, 0}, {15, 0}, {16, 0}, {17, 0}, {18, 0} };
    animation.frame_durations_ms = { 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100 };;
    animation.loop = true;
    animation.current_frame = 0;
    */
    Entity playerEntity = registry.players.entities[0]; // Assuming single player

    // Logic to decide when to shoot a guided missile
    boss.shootTimer += elapsed_ms/1000;
    if (boss.shootTimer >= boss.shootCooldown) {
        boss.shootTimer = 0.0f; // Reset the timer

        // Position at which the missile is spawned could be directly at the boss's location or adjusted as needed
        vec2 startPosition = motion.position;
        createBossGuidedMissile(renderer, startPosition, entity, playerEntity);
    }

    // Example transition logic back to another state, adjust according to your game design
    boss.stateTimer += elapsed_ms/1000;
    if (boss.stateTimer >= boss.stateDuration) {
        //boss.state = BossAI::BossState::DEFENSIVE; // Switch back to defensive or any other state
        boss.stateTimer = 0.0f;
    }
}

void Boss::updateGuidedMissiles(float elapsed_ms) {
    for (auto& missileEntity : registry.guidedMissiles.entities) { // Assuming you have a way to track guided missile entities
        Motion& missileMotion = registry.motions.get(missileEntity);
        Entity targetEntity = registry.players.entities[0]; // Assuming the player is the target
        Motion& targetMotion = registry.motions.get(targetEntity);

        // Calculate the direction to the target
        vec2 directionToTarget = normalize(targetMotion.position - missileMotion.position);

        // Adjust missile velocity to gradually steer towards the target
        float steeringStrength = 0.05f; // Control how sharply the missile can turn
        missileMotion.velocity = lerp(missileMotion.velocity, directionToTarget * glm::length(missileMotion.velocity), steeringStrength);

        // adjust the missile's rotation to face its direction of travel
        missileMotion.look_angle = atan2(missileMotion.velocity.y, missileMotion.velocity.x);
    }
}

vec2 Boss:: lerp(const glm::vec2& a, const glm::vec2& b, float t) {
    return a + t * (b - a);
}

