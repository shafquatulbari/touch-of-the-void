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
    if (!registry.rooms.has(registry.players.get(registry.players.entities[0]).current_room)) {
        return false; // Current room entity not found or does not have a Room component
    }

    const Room& room = registry.rooms.get(registry.players.get(registry.players.entities[0]).current_room);
    for (const auto& obstaclePos : room.obstacle_positions) {
        if (obstaclePos == position) {
            return true; // Found an obstacle at the given position
        }
    }
    return false; // No obstacle at the given position
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
    // Accumulators for the flocking behaviors
    vec2 alignment(0.0f, 0.0f); // Average direction
    vec2 cohesion(0.0f, 0.0f);  // Center of mass
    vec2 separation(0.0f, 0.0f); // Distance from each other

    int neighborCount = 0; // Number of nearby entities
    float perceptionRadius = 700.0f; // Adjusted for larger radius
    float separationDistance = 50.0f; // Adjusted for closer proximity
    float maxSpeed = 200.0f; // Adjusted for faster movement
    float maxForce = 0.1f; // Lower force for smoother steering

    // Loop through each entity to apply flocking behaviors
    for (auto& otherEntity : registry.ais.entities) {
        if (otherEntity == entity) continue; // Skip self

        Motion& otherMotion = registry.motions.get(otherEntity);
        float distance = length(motion.position - otherMotion.position);

        if (distance > 0 && distance < perceptionRadius) {
            // alignment: steer towards the average heading of local flockmates
            alignment += otherMotion.velocity;
            // cohesion: steer to move toward the average position of local flockmates
            cohesion += otherMotion.position;
            // separation: steer to avoid crowding local flockmates
            if (distance < separationDistance) {
                separation += (motion.position - otherMotion.position) / distance; // Weight by distance
            }
            neighborCount++;
        }
    }

    // average the accumulators and calculate steering vectors
    if (neighborCount > 0) {
        alignment /= neighborCount;
        cohesion /= neighborCount;
        cohesion -= motion.position; // steering vector towards the center of mass
        separation /= neighborCount;

        // Normalize and apply max force
        if (length(alignment) > 0) {
            alignment = normalize(alignment) * maxSpeed - motion.velocity;
            alignment = limit(alignment, maxForce);
        }
        if (length(cohesion) > 0) {
            cohesion = normalize(cohesion) * maxSpeed - motion.velocity;
            cohesion = limit(cohesion, maxForce);
        }
        if (length(separation) > 0) {
            separation = normalize(separation) * maxSpeed - motion.velocity;
            separation = limit(separation, maxForce);
        }
    }

    // Combine behaviors with weighted factors
    vec2 steer = alignment * 1.0f + cohesion * 1.0f + separation * 1.5f; // Increase separation weight slightly

    // Limit the final steering force and apply it to current velocity
    steer = limit(steer, maxForce);
    motion.velocity += steer;

    // Ensure the velocity stays within max speed
    motion.velocity = limit(motion.velocity, maxSpeed);

    return motion.velocity;
}

// Utility function to limit the magnitude of a vector to max
vec2 AISystem::limit(vec2 v, float max) {
    if (length(v) > max) {
        return normalize(v) * max;
    }
    return v;
}

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

    vec2 avoidanceForce(0.0f);
    float avoidanceRadius = 100.0f; // Example radius within which to avoid obstacles/projectiles

    const Room& room = registry.rooms.get(registry.players.get(registry.players.entities[0]).current_room);

    // Sample avoidance logic: Check for nearby obstacles and adjust direction
    for (const auto& obstaclePos : room.obstacle_positions) {
        if (glm::distance(motion.position, obstaclePos) < avoidanceRadius) {
			avoidanceForce += normalize(motion.position - obstaclePos);
		}
	}
    // Check for nearby projectiles and avoid them
    for (Entity projectile : registry.projectiles.entities) {
		vec2 position = registry.motions.get(entity).position;
		vec2 projectilePosition = registry.motions.get(projectile).position;
        if (glm::distance(position, projectilePosition) < avoidanceRadius) {
			avoidanceForce += normalize(position - projectilePosition);
		}
	}

    // Apply avoidance force to the current motion
    if (glm::length(avoidanceForce) > 0) {
        motion.velocity += normalize(avoidanceForce) * (100.0f * elapsed_ms);
    }

    switch (ai.type) {
    case AI::AIType::RANGED:
        handleRangedAI(entity, motion, ai, elapsed_ms, playerPosition);
        break;
    case AI::AIType::MELEE:
        handleMeleeAI(entity, motion, ai, elapsed_ms, playerPosition);
        break;
    default:
        break;
    }

    // Ensure velocity is clamped to avoid exceeding max speed after applying avoidance
    float maxSpeed = 50.0f; // Adjust as needed
    if (glm::length(motion.velocity) > maxSpeed) {
        motion.velocity = normalize(motion.velocity) * maxSpeed;
    }

    motion.position += motion.velocity * elapsed_ms / 1000.0f;
}

void AISystem::handleMeleeAI(Entity entity, Motion& motion, AI& ai, float elapsed_ms, const vec2& playerPosition) {
    // Check for line of sight
    if (lineOfSightClear(motion.position, playerPosition)) {
        // Generate path using A* if the line of sight is clear
        std::vector<vec2> path = findPathAStar(motion.position, playerPosition);
        if (!path.empty()) {
            vec2 nextStep = path[1]; // Get the next step towards the player
            // Calculate direction to the next step and update the enemy's velocity
            vec2 direction = normalize(nextStep - motion.position);
            //TUNE THIS VALUE
            float speed = 10.0f; 
            // Update velocity towards the next step in the path
            motion.velocity = direction * speed;
        }
    } else {
        // if line of sight is not clear
        motion.velocity = vec2(0.0f, 0.0f);
    }
}

void AISystem::handleRangedAI(Entity entity, Motion& motion, AI& ai, float elapsed_ms, const vec2& playerPosition) {
    float shootingRange = 300.0f; // Distance to shoot at the player
    float playerAvoidanceDistance = 100.0f; // Distance to keep from the player

    vec2 flockMove = flockMovement(entity, motion, elapsed_ms, playerPosition, playerAvoidanceDistance);

    // Calculate distance to the player
    float distanceToPlayer = length(playerPosition - motion.position);

    // Movement logic considering player's distance and projectiles
    if (distanceToPlayer < shootingRange) {
        if (distanceToPlayer > playerAvoidanceDistance) {
            // Move normally if within shooting range but outside avoidance distance
            motion.velocity = flockMove;
        }
        else {
            // Move away from the player if too close
            vec2 awayFromPlayer = normalize(motion.position - playerPosition) * 100.0f; // Strength of repulsion
            motion.velocity = flockMove + awayFromPlayer;
        }
    }
    else {
        // If player is too far, focus on flocking and avoiding obstacles
        motion.velocity = flockMove;
    }
    // Calculate the next position based on current velocity
    vec2 nextPosition = motion.position + (motion.velocity * elapsed_ms / 1000.0f);

    // Check if the next position is within bounds
    if (!isPositionWithinBounds(nextPosition)) {
        // Adjust velocity to bounce off the wall
        if (nextPosition.x <= xMin + 64.0f || nextPosition.x >= xMax - 64.0f) {
            motion.velocity.x *= -1; // Invert X component if hitting left or right wall
        }
        if (nextPosition.y <= yMin + 64.0f || nextPosition.y >= yMax - 64.0f) {
            motion.velocity.y *= -1; // Invert Y component if hitting top or bottom wall
        }
    }
    // Clamp velocity to max speed
    float maxSpeed = 50.0f;
    if (length(motion.velocity) > maxSpeed) {
        motion.velocity = normalize(motion.velocity) * maxSpeed;
    }

    // Shooting logic
    if (distanceToPlayer <= shootingRange) {
        ai.shootingCooldown -= elapsed_ms / 1000.0f; // Convert milliseconds to seconds
        if (ai.shootingCooldown <= 0) {
            vec2 shootingDirection = normalize(playerPosition - motion.position);
            float shootingAngle = atan2(shootingDirection.y, shootingDirection.x);
            createProjectileForEnemy(motion.position, shootingAngle, entity);
            ai.shootingCooldown = 2.5f; // Reset cooldown
        }
    }
}

bool AISystem::isPositionWithinBounds(const vec2& position) {
    // Using the game world boundaries 
    float leftBound = xMin + 64.0f; 
    float rightBound = xMax - 64.0f; 
    float topBound = yMin + 64.0f;
    float bottomBound = yMax - 64.0f; 

    // Check if the position is within bounds
    return position.x >= leftBound && position.x <= rightBound &&
        position.y >= topBound && position.y <= bottomBound;
}

// Create projectile for enemy
void AISystem::createProjectileForEnemy(vec2 position, float angle, Entity source) {
    // Adjust angle, position, and possibly texture for the enemy's projectiles
    float rng = 0.0f; // Assuming no randomness for enemy shots, adjust as needed
    float fire_length = 0.0f; // Not used for enemies in this context
    createProjectile(renderer, position, angle, rng, fire_length, source);
}
