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

struct Vec2Hash {
    std::size_t operator()(const vec2& v) const {
        return std::hash<float>()(v.x) ^ std::hash<float>()(v.y);
    }
};

bool AISystem::lineOfSightClear(const vec2& start, const vec2& end) {
    vec2 direction = normalize(end - start);
    float distance = length(end - start);
    float step = 10.0f; // Adjust based on your needs

    for (float t = 0.0f; t <= distance; t += step) {
        vec2 currentPosition = start + direction * t;
        if (isObstacleAtPosition(currentPosition)) {
            return true; // note set it back to false
        }
    }

    return true;
}

// Adjusted obstacle detection to consider the full bounding box of obstacles
bool AISystem::isObstacleAtPosition(const vec2& position) {
    // Iterate through all obstacles in the game world
    for (const auto& obstacleEntity : registry.obstacles.entities) {
        const Motion& obstacle = registry.motions.get(obstacleEntity);

        // Calculate the obstacle's bounding box in world coordinates
        vec2 topLeft = obstacle.position - (obstacle.scale / 2.0f);
        vec2 bottomRight = obstacle.position + (obstacle.scale / 2.0f);

        // Check if the given position falls within the obstacle's bounding box
        if (position.x >= topLeft.x && position.x <= bottomRight.x &&
            position.y >= topLeft.y && position.y <= bottomRight.y) {
            return false; // The position intersects with an obstacle , note: set it back to true
        }
    }

    return false; // No collision detected
}

// Basic structure for A* nodes
struct AStarNode {
    vec2 position;
    float gCost; // Cost from start to this node
    float hCost; // Heuristic cost to goal
    float fCost() const { return gCost + hCost; } // Total cost
    AStarNode* parent; // For reconstructing the path
};

struct CompareAStarNode {
    bool operator()(AStarNode* a, AStarNode* b) const {
        return a->fCost() > b->fCost();
    }
};
std::vector<vec2> AISystem::generateNeighbors(const vec2& position) {
    std::vector<vec2> neighbors;
    //generate neighbors in four directions
    float stepSize = 10.0; // adjust as necessary
    neighbors.push_back(position + vec2(stepSize, 0)); // Right
    neighbors.push_back(position + vec2(-stepSize, 0)); // Left
    neighbors.push_back(position + vec2(0, stepSize)); // Up
    neighbors.push_back(position + vec2(0, -stepSize)); // Down
    // Add more directions if needed
    return neighbors;
}


std::vector<vec2> AISystem::findPathAStar(const vec2& start, const vec2& goal) {
    auto heuristic = [](const vec2& a, const vec2& b) -> float {
        // Use Euclidean distance as the heuristic
        return length(a - b);
        };

    std::priority_queue<AStarNode*, std::vector<AStarNode*>, CompareAStarNode> openSet;
    std::unordered_map<vec2, AStarNode*, Vec2Hash> allNodes;
    std::vector<vec2> path;

    AStarNode* startNode = new AStarNode{ start, 0, heuristic(start, goal), nullptr };
    openSet.push(startNode);
    allNodes[start] = startNode;

    while (!openSet.empty()) {
        AStarNode* currentNode = openSet.top();
        openSet.pop();

        if (length(currentNode->position - goal) < 10.0f) { // Threshold to consider as goal reached
            while (currentNode != nullptr) {
                path.push_back(currentNode->position);
                currentNode = currentNode->parent;
            }
            std::reverse(path.begin(), path.end());
            break;
        }

        std::vector<vec2> neighbors = generateNeighbors(currentNode->position);

        for (auto& neighborPos : neighbors) {
            if (isObstacleAtPosition(neighborPos)) continue;

            float tentative_gScore = currentNode->gCost + length(neighborPos - currentNode->position);
            AStarNode* neighborNode = allNodes.find(neighborPos) != allNodes.end() ? allNodes[neighborPos] : nullptr;

            if (neighborNode == nullptr || tentative_gScore < neighborNode->gCost) {
                if (neighborNode == nullptr) {
                    neighborNode = new AStarNode{ neighborPos, tentative_gScore, heuristic(neighborPos, goal), currentNode };
                    allNodes[neighborPos] = neighborNode;
                    openSet.push(neighborNode);
                }
                else {
                    neighborNode->gCost = tentative_gScore;
                    neighborNode->parent = currentNode;
                }
            }
        }
    }

    for (auto& nodePair : allNodes) {
        delete nodePair.second;
    }
    allNodes.clear();

    return path;
}


//For now just implement BFS so that enemy of type Melee can follow the player
void AISystem::step(float elapsed_ms)
{
	auto& ai_registry = registry.ais;
    for (uint i = 0; i < ai_registry.size(); i++)
	{
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

    // Check for line of sight for both ranged and melee types
    if (lineOfSightClear(motion.position, playerPosition)) {
        // Rotate towards player for both melee and ranged if in line of sight
        vec2 direction = normalize(playerPosition - motion.position);
        float angle = atan2(direction.y, direction.x);
        motion.look_angle = angle; // Rotate towards player

        AI& ai = registry.ais.get(entity);

        if (ai.type == AI::AIType::RANGED) {
            // Ranged AI behavior
            handleRangedAI(entity, motion, ai, elapsed_ms, playerPosition);
        }
        else if (ai.type == AI::AIType::MELEE) {
            // Melee AI behavior using A* for pathfinding
            std::vector<vec2> path = findPathAStar(motion.position, playerPosition);
            if (path.size() > 1) { // Ensure path has more than just the starting position
                vec2 nextStep = path[1]; // Get the next step towards the player

                direction = normalize(nextStep - motion.position);
                motion.look_angle = atan2(direction.y, direction.x);
                float speed = 10.0f; // Define a suitable speed for the melee enemy

                // Update velocity towards the next step in the path
                motion.velocity = direction * speed;
            }
        }
    }
    else {
        // If there's no line of sight, stop movement
        motion.velocity = vec2(0.0f, 0.0f);
    }
}

void AISystem::handleRangedAI(Entity entity, Motion& motion, AI& ai, float elapsed_ms, const vec2& playerPosition) {
    float distanceToPlayer = length(playerPosition - motion.position);

    if (lineOfSightClear(motion.position, playerPosition)) {
        // Rotate towards the player
        vec2 direction = normalize(playerPosition - motion.position);
        motion.look_angle = atan2(direction.y, direction.x);

        if (distanceToPlayer > ai.safe_distance) {
            // Continue moving closer until within a safe distance, then stop
            motion.velocity = direction * 10.0f; // Assume 10.0f is a speed value for the AI
        }
        else {
            // Within safe distance, stop moving
            motion.velocity = vec2(0.0f, 0.0f);

            // Shooting logic with cooldown
            ai.shootingCooldown -= elapsed_ms / 1000.0f; // Convert milliseconds to seconds
            if (ai.shootingCooldown <= 0) {
                createProjectileForEnemy(motion.position, motion.look_angle, entity);
                ai.shootingCooldown = 2.5f; // Reset cooldown to 2.5 seconds or appropriate value
            }
        }
    }
    else {
        // No line of sight to the player, stop moving
        motion.velocity = vec2(0.0f, 0.0f);
    }
}

// Create projectile for enemy
void AISystem::createProjectileForEnemy(vec2 position, float angle, Entity source) {
    // Utilize createProjectile with adjustments for the enemy
    // Adjust angle, position, and possibly texture for the enemy's projectiles
    float rng = 0.0f; // Assuming no randomness for enemy shots, adjust as needed
    float fire_length = 0.0f; // Not used for enemies in this context
    createProjectile(renderer, position, angle, rng, fire_length, source);
}
