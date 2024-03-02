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

const int WIDTH = 480;
const int HEIGHT = 32;

const int GRID_SIZE_X = 480; // according to your game's layout
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
            case AI::AIState::ATTACK:
                printf("Attack state\n");
                attackingState(entity, motion, elapsed_ms);
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
    
}

void AISystem::activeState(Entity entity, Motion& motion, float elapsed_ms) {
    vec2 playerPosition = registry.motions.get(registry.players.entities[0]).position;
    std::vector<vec2> path = findPath(motion.position, playerPosition);

    if (!path.empty() && path.size() > 1) { // Path includes start and next step at least
        vec2 nextStep = path[1]; // Assuming path[0] is the current position
        vec2 direction = normalize(nextStep - motion.position);
        float speed = 10.0f; // Define a suitable speed value for your game
        motion.velocity = direction * speed;
        motion.position += motion.velocity * (elapsed_ms / 1000.0f); // Apply movement
    }
}


void AISystem::attackingState(Entity entity, Motion& motion, float elapsed_ms) {
    // Attack behavior (e.g., stop moving, play attack animation, deal damage)
    // ... Logic for attacks and handling damage to the player 
}


vec2 AISystem::worldToGrid(const vec2& pos) {
    return vec2(round(pos.x / (window_width_px / WIDTH)), round(pos.y / (window_height_px / HEIGHT)));
}

vec2 AISystem::gridToWorld(const vec2& gridPos) {
    return vec2(gridPos.x * (window_width_px / WIDTH), gridPos.y * (window_height_px / HEIGHT));
}
void AISystem::updateGrid() {
    // Reset grid
    for (auto& row : grid) std::fill(row.begin(), row.end(), true);

    // Mark obstacles in the grid
    for (const auto& obstaclePos : registry.rooms.get(registry.rooms.entities[0]).obstacle_positions) {
        vec2 gridPos = worldToGrid(obstaclePos);
        if (gridPos.x >= 0 && gridPos.x < GRID_SIZE_X && gridPos.y >= 0 && gridPos.y < GRID_SIZE_Y) {
            grid[static_cast<int>(gridPos.x)][static_cast<int>(gridPos.y)] = false; // Mark as non-walkable
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
