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

        // Check AI state and perform actions accordingly
        switch (boss.state) {
            case BossAI::BossState::DEFENSIVE:
				handleDefensiveState(entity, boss, motion, elapsed_ms);
				break;
            case BossAI::BossState::OFFENSIVE:
                handleOffensiveState(entity, boss, motion, elapsed_ms);
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

    // Check if it's time to switch state
    if (boss.stateTimer >= boss.stateDuration) {
        boss.stateTimer = 0.0f; // Reset the state timer
        // Switch state
        boss.state = BossAI::BossState::OFFENSIVE;
    }
}

void Boss::handleOffensiveState(Entity entity, BossAI& boss, Motion& motion, float elapsed_ms)
{
    // Specify types for each enemy, later need to find a way to assign types randomly now its 2 ranged 1 melee
    std::vector<AI::AIType> enemy_types = { AI::AIType::MELEE };
    //enemy positions is a set of vec2
    vec2 pos = { 7.0f, 3.0f }; 
    float x_origin = (window_width_px / 2) - (game_window_size_px / 2) + 32;
    float y_origin = (window_height_px / 2) - (game_window_size_px / 2) + 32;
    float x = x_origin + pos.x * game_window_block_size;
    float y = y_origin + pos.y * game_window_block_size;
    // Adjust state duration for offensive state
    boss.stateDuration = 15.0f;

    // Generate enemies only when entering the state
    if (boss.stateTimer == 0.0f) {
        int num_enemies = rand() % 3 + 4; // Generate between 4 to 6 enemies
        boss.aliveEnemyCount = num_enemies;
        for (int i = 0; i < 3; i++) {
            // Generate enemies at random positions around the boss
            // Placeholder for enemy generation code
            // Adjust the `createEnemy` function call as needed based on your game's logic
            createEnemy(renderer, vec2(x, y), 500.0f, enemy_types[rand() % enemy_types.size()], true);
        }
    }

    // Check if it's time to switch state and if all enemies are defeated
    if (boss.stateTimer >= boss.stateDuration && boss.aliveEnemyCount == 0) {
        boss.stateTimer = 0.0f; // Reset the state timer
        boss.state = BossAI::BossState::DEFENSIVE; // Switch to defensive state
    }
    else {
        boss.stateTimer += elapsed_ms / 1000.0f; // Update the state timer only if not switching
    }
    
}