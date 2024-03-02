// internal
#include "ai_system/ai_system.hpp"
#include <world_system/world_system.hpp>
#include <queue> // For priority queue (open list)
#include <unordered_set>  
#include <algorithm> 
#include <vector>
#include <cmath>
#include <memory>
#include <map>
#include <world_init/world_init.hpp>

const int WIDTH = 32;
const int HEIGHT = 32;

const int GRID_SIZE_X = 32; // according to your game's layout
const int GRID_SIZE_Y = 32; // according to your game's layout
std::vector<std::vector<bool>> grid(GRID_SIZE_X, std::vector<bool>(GRID_SIZE_Y, true)); // true for walkable, false for blocked

struct Vec2Hash {
    std::size_t operator()(const vec2& v) const {
        return std::hash<float>()(v.x) ^ std::hash<float>()(v.y);
    }
};

//For now just implement BFS so that enemy of type Melee can follow the player
void AISystem::step(float elapsed_ms)
{
	auto& ai_registry = registry.ais;
    for (uint i = 0; i < ai_registry.size(); i++)
	{
        updateGrid();
        /*
        // Debug print the grid after updating it
        for (int i = 0; i < GRID_SIZE_X; i++) {
            for (int j = 0; j < GRID_SIZE_Y; j++) {
                if (!grid[i][j]) {
                    printf("Obstacle at grid position: (%d, %d)\n", i, j);
                }
            }
            printf("\n"); // New line for each row for better visibility
        }
        */
        AI& ai = ai_registry.components[i];
		Entity entity = ai_registry.entities[i];
		         
            Motion& motion = registry.motions.get(entity);
            // State machine for AI behavior
            switch (ai.state) {
                printf("State machine\n");
            case AI::AIState::IDLE:
                printf("Idle state\n");
                idleState(entity, motion);
                break;
            case AI::AIState::ACTIVE:
                //printf("Active state\n");
                activeState(entity, motion, elapsed_ms);
                break;
            default:
                printf("Default state\n");
                break;
            }
            motion.position += motion.velocity * elapsed_ms / 1000.0f;
            
        }
       
        // Update position based on velocity (assuming physics system handles integration)
       
}

void AISystem:: idleState(Entity entity, Motion& motion) {
	// No movement in idle state
	motion.velocity = vec2(0.0f, 0.0f);
}

void AISystem::activeState(Entity entity, Motion& motion, float elapsed_ms) {
    vec2 playerPosition = registry.motions.get(registry.players.entities[0]).position;
    std::vector<vec2> path = findPath(motion.position, playerPosition);
    AI& ai = registry.ais.get(entity);

    if (ai.type == AI::AIType::RANGED) {
        handleRangedAI(entity, motion, ai, elapsed_ms);
    }
    else if (ai.type == AI::AIType::MELEE) {
        if (!path.empty() && path.size() > 1) {
            vec2 nextStep = path[1]; // Assuming path[0] is the current position
            vec2 direction = normalize(nextStep - motion.position);
            float speed = 10.0f; // Define a suitable speed value for your game

            vec2 gridPos = worldToGrid(nextStep);
            if (grid[static_cast<int>(gridPos.x)][static_cast<int>(gridPos.y)]) {
                motion.velocity = direction * speed;
                motion.position += motion.velocity * (elapsed_ms / 1000.0f); // Corrected application
            }
            else {
                motion.velocity = vec2(0.0f, 0.0f); // Stop movement if next step is not walkable
            }
        }
	}
}

//Handle ranged AI behavior
void AISystem::handleRangedAI(Entity entity, Motion& motion, AI& ai, float elapsed_ms) {
    vec2 playerPosition = registry.motions.get(registry.players.entities[0]).position;

    if (lineOfSightClear(motion.position, playerPosition)) {
        // There's a clear line of sight to the player, rotate and shoot
        vec2 direction = normalize(playerPosition - motion.position);
        float angle = atan2(direction.y, direction.x);

        // Rotate towards player
        motion.look_angle = angle;

        // Shoot projectile towards player
        createProjectileForEnemy(motion.position, angle, entity); // Assuming createProjectileForEnemy takes the enemy as source
    }
}

// A simple algorithm to check for line of sight
bool AISystem::lineOfSightClear(const vec2& start, const vec2& end) {
    // Normalize direction for consistent stepping
    vec2 direction = normalize(end - start);
    float distance = length(end - start);

    // Reduce step size for higher precision, it should be small enough to catch all grid cells
    float step = game_window_block_size * 0.1f; // Step size set to a fraction of the block size

    vec2 currentPosition = start;
    for (float i = 0; i <= distance; i += step) {
        currentPosition += direction * step; // move the current position
        vec2 gridPos = worldToGrid(currentPosition); // convert to grid position

        // Check grid bounds
        if (gridPos.x < 0 || gridPos.x >= GRID_SIZE_X || gridPos.y < 0 || gridPos.y >= GRID_SIZE_Y) {
            return false; // Out of grid bounds
        }

        // Check if there's an obstacle
        if (!grid[static_cast<int>(gridPos.x)][static_cast<int>(gridPos.y)]) {
            return false; // Obstacle detected
        }
    }

    return true; // No obstacle detected in line of sight
}

// Create projectile for enemy
void AISystem::createProjectileForEnemy(vec2 position, float angle, Entity source) {
    // Utilize createProjectile with adjustments for the enemy
    // Adjust angle, position, and possibly texture for the enemy's projectiles
    float rng = 0.0f; // Assuming no randomness for enemy shots, adjust as needed
    float fire_length = 0.0f; // Not used for enemies in this context
    createProjectile(nullptr, position, angle, rng, fire_length, source);
}

vec2 AISystem::worldToGrid(const vec2& pos) {
    // Offset the position by the game window origin before dividing by block size
    float x_offset = (window_width_px / 2) - (game_window_size_px / 2);
    float y_offset = (window_height_px / 2) - (game_window_size_px / 2);
    int gridX = static_cast<int>((pos.x - x_offset) / game_window_block_size);
    int gridY = static_cast<int>((pos.y - y_offset) / game_window_block_size);
    return vec2(gridX, gridY);
}

vec2 AISystem::gridToWorld(const vec2& gridPos) {
    // Add the game window origin to the block position
    float x_offset = (window_width_px / 2) - (game_window_size_px / 2);
    float y_offset = (window_height_px / 2) - (game_window_size_px / 2);
    float x = gridPos.x * game_window_block_size + x_offset + game_window_block_size / 2.0f;
    float y = gridPos.y * game_window_block_size + y_offset + game_window_block_size / 2.0f;
    return vec2(x, y);
}

void AISystem::updateGrid() {
    // Reset grid
    for (auto& row : grid) std::fill(row.begin(), row.end(), true);

    // Mark obstacles 
    for (Entity entity : registry.obstacles.entities) {
        Motion& motion = registry.motions.get(entity);
        vec2 gridPos = worldToGrid(motion.position);

        // Bounds Check
        if (gridPos.x >= 0 && gridPos.x < GRID_SIZE_X && gridPos.y >= 0 && gridPos.y < GRID_SIZE_Y) {
            grid[static_cast<int>(gridPos.x)][static_cast<int>(gridPos.y)] = false;
        }
    }
}

// using BFS to find the shortest path
std::vector<vec2> AISystem::findPath(const vec2& startWorld, const vec2& goalWorld) {
    vec2 start = worldToGrid(startWorld);
    vec2 goal = worldToGrid(goalWorld);

    std::queue<vec2> queue;
    std::unordered_map<vec2, vec2, Vec2Hash> cameFrom;
    std::vector<vec2> path;

    queue.push(start);
    cameFrom[start] = start;

    while (!queue.empty()) {
        vec2 current = queue.front();
        queue.pop();

        if (current == goal) {
            while (current != start) {
                path.push_back(gridToWorld(current));
                current = cameFrom[current];
            }
            path.push_back(gridToWorld(start));
            std::reverse(path.begin(), path.end());
            return path;
        }

        for (vec2 next : {vec2(0, 1), vec2(1, 0), vec2(0, -1), vec2(-1, 0)}) {
            vec2 neighbor = current + next;
            if (cameFrom.find(neighbor) == cameFrom.end() && neighbor.x >= 0 && neighbor.x < GRID_SIZE_X && neighbor.y >= 0 && neighbor.y < GRID_SIZE_Y && grid[neighbor.x][neighbor.y]) {
                queue.push(neighbor);
                cameFrom[neighbor] = current;
            }
        }
    }

    return path; // Return empty path if goal is not reachable
}
