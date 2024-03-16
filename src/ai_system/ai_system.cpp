// internal
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

// coordinate system details
// world is 960x960
// each cell is 64x64 so there are 15x15 cells
// the top left cell is (0,0) and the bottom right cell is (14,14)
// the world is placed in the middle of the screen (where the screen is 1920x1024)
// so the top left corner of the world is at (480, 32) and the bottom right corner is at (1440, 992)
const float cellSize = 64.0f;
const int numCells = 15;
const int maxIterations = 1000;
const float xMin = 480.0f; 
const float yMin = 32.0f;
const float xMax = 1440.0f;
const float yMax = 992.0f;

struct Vec2Hash {
    std::size_t operator()(const vec2& v) const {
        return std::hash<float>()(v.x) ^ std::hash<float>()(v.y);
    }
};

bool AISystem::lineOfSightClear(const vec2& start, const vec2& end) {
    // http://www.cse.yorku.ca/~amana/research/grid.pdf
    // Amanatides, J., & Woo, A. (1987). A fast voxel traversal algorithm for ray tracing. Eurographics, 87(3), 3-10.
    // This is a simple implementation of the Bresenham's line algorithm

    int x = static_cast<int>(start.x - xMin);
    int cellX = static_cast<int>(floor((x / cellSize)));
    int y = static_cast<int>(start.y - yMin);
    int cellY = static_cast<int>(floor((y/ cellSize)));
    assert(xMin <= x <= xMax && yMin <= y <= yMax);
    assert(0 <= cellX <= numCells - 1 && 0 <= cellY <= numCells - 1);

    int endX = static_cast<int>(end.x - xMin);
    int endCellX = static_cast<int>(floor((endX / cellSize)));
    int endY = static_cast<int>(end.y - yMin);
    int endCellY = static_cast<int>(floor((endY / cellSize)));
    assert(xMin <= endX <= xMax && yMin <= endY <= yMax);
    assert(0 <= endCellX <= numCells - 1 && 0 <= endCellY <= numCells - 1);

    vec2 direction = end - start;
    int stepX = (direction.x > 0) ? 1 : -1;
    int stepY = (direction.y > 0) ? 1 : -1;

    float deltaT_X = fabs(cellSize / direction[0]);
    float deltaT_Y = fabs(cellSize / direction[1]);

    float remainingT_X = deltaT_X * (1 - fmod(start.x, cellSize));
    float remainingT_Y = deltaT_Y * (1 - fmod(start.y, cellSize));

   for (int i = 0; i < maxIterations; i++) {
		if (remainingT_X < remainingT_Y) {
            cellX += stepX;
			remainingT_X += deltaT_X;
		}
		else {
            cellY += stepY;
			remainingT_Y += deltaT_Y;
		}

        cellX = std::max(0, std::min(cellX, numCells - 1));
        cellY = std::max(0, std::min(cellY, numCells - 1));

        if (isObstacleAtPosition(vec2(cellX, cellY))) {
            return false;
        }

        if (cellX == endCellX && cellY == endCellY) {
            break;
		}
	}
   return true;
}

// Adjusted obstacle detection to consider the full bounding box of obstacles
bool AISystem::isObstacleAtPosition(const vec2& position) {
    // position is in grid coordinates
    // check if there is an obstacle at the given position
    // NEED A WAY TO BE TOLD THE CURENT ROOM
    // once has would something along these lines
   /* Room& room = registry.rooms.get(currentRoom);

    for (auto& pos : room.obstacle_positions) {
        if (pos.x == position.x && pos.y == position.y) {
            return true;
        };
	}*/

    return false;
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

// https://en.wikipedia.org/wiki/Boids
//Reynolds, C. W. (1987). Flocks, herds and schools: A distributed behavioral model. 
// In Proceedings of the 14th annual conference on Computer graphics and interactive techniques (pp. 25-34).
// The Boids algorithm for flocking behavior
vec2 AISystem::flockMovement(Entity entity, Motion& motion, float elapsed_ms, const vec2& playerPosition, float playerAvoidanceDistance) {
    vec2 alignment = vec2(0.0f, 0.0f); // Direction that aligns with neighboring boids
    vec2 cohesion = vec2(0.0f, 0.0f);  // Direction towards the center of mass of neighboring boids
    vec2 separation = vec2(0.0f, 0.0f); // Direction to maintain a minimum distance from neighboring boids
    vec2 avoidance = vec2(0.0f, 0.0f); // Direction to avoid projectiles

    int neighborCount = 0;
    //These are the tuning parameters
    float maxSpeed = 100.0f;
    float perceptionRadius = 150.0f;
    float separationDistance = 100.0f;

    // Loop through all entities to apply alignment, cohesion, and separation
    for (auto& otherEntity : registry.ais.entities) {
        if (otherEntity == entity) continue; // Skip self

        Motion& otherMotion = registry.motions.get(otherEntity);
        vec2 direction = otherMotion.position - motion.position;
        float distance = length(direction);

        if (distance < perceptionRadius && distance > 0) {
            alignment += otherMotion.velocity;
            cohesion += otherMotion.position;

            if (distance < separationDistance) {
                separation -= (direction / distance) * (separationDistance - distance);
            }

            neighborCount++;
        }
    }

    // Normalize and apply maxSpeed to alignment, cohesion, and separation
    if (neighborCount > 0) {
        alignment = (normalize(alignment) * maxSpeed - motion.velocity) / 8.0f; // Tweaking the factor for smoothness
        cohesion = (normalize((cohesion / (float)neighborCount) - motion.position) * maxSpeed - motion.velocity) / 8.0f;
        separation = (separation / (float)neighborCount) * maxSpeed;
    }

    // Avoidance behavior for projectiles
    for (auto& projectileEntity : registry.projectiles.entities) {
        Motion& projectileMotion = registry.motions.get(projectileEntity);
        vec2 directionToProjectile = projectileMotion.position - motion.position;
        float distanceToProjectile = length(directionToProjectile);

        if (distanceToProjectile < perceptionRadius && distanceToProjectile > 0) {
            avoidance -= (directionToProjectile / distanceToProjectile) * maxSpeed;
        }
    }

    // Calculate avoidance vector for the player
    vec2 directionToPlayer = motion.position - playerPosition;
    float distanceToPlayer = length(directionToPlayer);

    if (distanceToPlayer < playerAvoidanceDistance && distanceToPlayer > 0) {
        avoidance += (directionToPlayer / distanceToPlayer) * maxSpeed;
    }

    // Combine the steering vectors
    vec2 steer = alignment + cohesion + separation + avoidance;

    // Ensure the speed does not exceed maxSpeed
    if (length(steer) > maxSpeed) {
        steer = normalize(steer) * maxSpeed;
    }

    return steer;
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

    //// Check for line of sight for both ranged and melee types
    //if (lineOfSightClear(motion.position, playerPosition)) {
    //    // Rotate towards player for both melee and ranged if in line of sight
    //    vec2 direction = normalize(playerPosition - motion.position);
    //    float angle = atan2(direction.y, direction.x);
    //    motion.look_angle = angle; // Rotate towards player

    AI& ai = registry.ais.get(entity);
    switch (ai.type) {
        case AI::AIType::RANGED:
            // Ranged AI behavior
            handleRangedAI(entity, motion, ai, elapsed_ms, playerPosition);
            break;
        case AI::AIType::MELEE:
            //handleMeleeAI(entity, motion, ai, elapsed_ms, playerPosition);
            // Melee AI behavior using A* for pathfinding
            //    std::vector<vec2> path = findPathAStar(motion.position, playerPosition);
            //    if (path.size() > 1) { // Ensure path has more than just the starting position
            //        vec2 nextStep = path[1]; // Get the next step towards the player

            //        direction = normalize(nextStep - motion.position);
            //        motion.look_angle = atan2(direction.y, direction.x);
            //        float speed = 10.0f; // Define a suitable speed for the melee enemy

            //        // Update velocity towards the next step in the path
            //        motion.velocity = direction * speed;
            //    }
            break;
        default:
            break;
    }
}

void AISystem::handleRangedAI(Entity entity, Motion& motion, AI& ai, float elapsed_ms, const vec2& playerPosition) {

    float playerAvoidanceDistance = 200.0f; // Adjust as necessary

    // Calculate the flock movement vector for the current entity
    vec2 flockMove = flockMovement(entity, motion, elapsed_ms, playerPosition, playerAvoidanceDistance);

    float maxSpeed = 100.0f; // Adjust as necessary

    // Calculate distance to the player
    float distanceToPlayer = length(playerPosition - motion.position);

    // Decide on the movement: flocking behavior but also maintain a safe distance from the player
    if (distanceToPlayer > ai.safe_distance) {
        // If too far from the player, move closer using the flocking behavior but also towards the player
        vec2 towardsPlayer = normalize(playerPosition - motion.position);
        // Mix the flockMove with some movement towards the player to ensure enemies don't stray too far
        motion.velocity = flockMove + towardsPlayer * (maxSpeed * 0.5f); // Adjust the weight as necessary
    }
    else if (distanceToPlayer < ai.safe_distance * 0.5f) {
        // If too close to the player, move away
        vec2 awayFromPlayer = normalize(motion.position - playerPosition);
        motion.velocity = awayFromPlayer * maxSpeed;
    }
    else {
        // Maintain current behavior and distance
        motion.velocity = flockMove;
    }

    // Limit the velocity to the maxSpeed
    if (length(motion.velocity) > maxSpeed) {
        motion.velocity = normalize(motion.velocity) * maxSpeed;
    }

    // Shooting logic
    ai.shootingCooldown -= elapsed_ms / 1000.0f; // Convert milliseconds to seconds
    if (ai.shootingCooldown <= 0 && distanceToPlayer <= ai.safe_distance * 1.5f) {
        // Enemy shoots towards the player if within a certain range and cooldown has passed
        vec2 shootingDirection = normalize(playerPosition - motion.position);
        float shootingAngle = atan2(shootingDirection.y, shootingDirection.x);
        createProjectileForEnemy(motion.position, shootingAngle, entity);
        ai.shootingCooldown = 2.5f; // Reset cooldown, adjust as necessary
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
