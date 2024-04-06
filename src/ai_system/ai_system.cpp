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
const float halfCellSize = cellSize / 2.0f;

struct Vec2Hash {
    std::size_t operator()(const vec2& v) const {
        return std::hash<float>()(v.x) ^ std::hash<float>()(v.y);
    }
};

vec2 AISystem::clampPositionToBounds(const vec2& position) {
    float clampedX = std::max(xMin + (cellSize + halfCellSize), std::min(position.x, xMax - (cellSize + halfCellSize)));
    float clampedY = std::max(yMin + (cellSize + halfCellSize), std::min(position.y, yMax - (cellSize + halfCellSize)));
    return vec2(clampedX, clampedY);
}

// http://www.cse.yorku.ca/~amana/research/grid.pdf
 // Amanatides, J., & Woo, A. (1987). A fast voxel traversal algorithm for ray tracing. Eurographics, 87(3), 3-10.
 // This is a simple implementation of the Bresenham's line algorithm
bool AISystem::lineOfSightClear(const vec2& start, const vec2& end) {
    // Convert start and end positions from world coordinates to grid coordinates
    auto toGridCoord = [&](const vec2& pos) -> vec2 {
        return vec2(floor((pos.x - xMin) / cellSize), floor((pos.y - yMin) / cellSize));
        };

    vec2 startGrid = toGridCoord(start);
    vec2 endGrid = toGridCoord(end);

    // Bresenham's Line Algorithm variables
    int dx = abs(int(endGrid.x - startGrid.x));
    int sx = startGrid.x < endGrid.x ? 1 : -1;
    int dy = -abs(int(endGrid.y - startGrid.y));
    int sy = startGrid.y < endGrid.y ? 1 : -1;
    int err = dx + dy; // error value e_xy
    int e2; 

    while (true) {
        // Check for obstacle at current grid position
        if (isObstacleAtPosition(vec2(startGrid.x, startGrid.y))) {
            return false; // Obstacle found, line of sight is not clear
        }
        if (startGrid.x == endGrid.x && startGrid.y == endGrid.y) break; // Reached target cell

        e2 = 2 * err;
        if (e2 >= dy) { // e_xy+e_x > 0
            err += dy;
            startGrid.x += sx;
        }
        if (e2 <= dx) { // e_xy+e_y < 0
            err += dx;
            startGrid.y += sy;
        }
    }

    return true; // No obstacles found, line of sight is clear
}


// Adjusted obstacle detection to consider the full bounding box of obstacles
bool AISystem::isObstacleAtPosition(const vec2& gridPosition) {
    Level& level = registry.levels.get(registry.levels.entities[0]);
    Room& room = registry.rooms.get(level.rooms[level.current_room]);

    for (const auto& obstaclePos : room.obstacle_positions) {
        // Direct comparison, as both are in grid coordinates
        if (obstaclePos == gridPosition) {
            return true; // Obstacle found at the specified grid position
        }
    }

    return false; // No obstacle at the specified grid position
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
    neighbors.push_back(position + vec2(stepSize, stepSize)); // Up-right
    neighbors.push_back(position + vec2(-stepSize, stepSize)); // Up-left
    neighbors.push_back(position + vec2(stepSize, -stepSize)); // Down-right
    neighbors.push_back(position + vec2(-stepSize, -stepSize)); // Down-left

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

        // Check AI state and perform actions accordingly
        switch (ai.state) {
        case AI::AIState::IDLE:
            idleState(entity, ai, motion, elapsed_ms); // Now passing elapsed_ms
            break;
        case AI::AIState::ACTIVE:
            activeState(entity, motion, elapsed_ms);
            break;
        default:
            printf("Unknown state\n");
            break;
        }

        // Always update position based on velocity, regardless of state
        motion.position += motion.velocity * elapsed_ms / 1000.0f;
    }
}

void AISystem::idleState(Entity entity, AI& ai, Motion& motion, float elapsed_ms) {
    // Example condition to switch to active state based on a simple timer
    ai.counter += elapsed_ms;
    if (ai.counter >= ai.frequency) {
        ai.counter = 0;
        ai.state = AI::AIState::ACTIVE;
    }

    // Optionally, apply some basic motion or behavior even in idle state
    // For example, slowing down or stopping:
    motion.velocity *= 0.95f; // Slow down effect
}

void AISystem::activeState(Entity entity, Motion& motion, float elapsed_ms) {
    vec2 playerPosition = registry.motions.get(registry.players.entities[0]).position;
    AI& ai = registry.ais.get(entity);

    switch (ai.type) {
    case AI::AIType::RANGED:
        handleRangedAI(entity, motion, ai, elapsed_ms, playerPosition);
        break;
    case AI::AIType::MELEE:
        handleMeleeAI(entity, motion, ai, elapsed_ms, playerPosition);
        break;
    case AI::AIType::TURRET:
        handleTurretAI(entity, motion, ai, elapsed_ms, playerPosition);
        break;
    case AI::AIType::SHOTGUN:
        handleShotgunAI(entity, motion, ai, elapsed_ms, playerPosition);
    case AI::AIType::ROCKET:
		handleRocketAI(entity, motion, ai, elapsed_ms, playerPosition);
    case AI::AIType::FLAMETHROWER:
		handleFlameAI(entity, motion, ai, elapsed_ms, playerPosition);
    default:
        break;
    }

    // Ensure velocity is clamped to avoid exceeding max speed after applying avoidance
    float maxSpeed = 50.0f; // Adjust as needed
    if (glm::length(motion.velocity) > maxSpeed) {
        motion.velocity = normalize(motion.velocity) * maxSpeed;
    }

    // Calculate the next position based on current velocity
    vec2 nextPosition = motion.position + (motion.velocity * elapsed_ms / 1000.0f);

    // Clamp the next position to ensure it's within bounds
    vec2 clampedPosition = clampPositionToBounds(nextPosition);

    // Update the entity's position to the clamped position
    motion.position = clampedPosition;
}

void AISystem::handleMeleeAI(Entity entity, Motion& motion, AI& ai, float elapsed_ms, const vec2& playerPosition) {
    // Check for line of sight
    if (lineOfSightClear(motion.position, playerPosition)) {
        // Rotate towards player if in line of sight
        vec2 direction = normalize(playerPosition - motion.position);
        float angle = atan2(direction.y, direction.x);
        motion.look_angle = angle;

        // Generate path using A* if the line of sight is clear
        std::vector<vec2> path = findPathAStar(motion.position, playerPosition);
        // Ensure there is a path and it has more than one point (start point is always included)
        if (path.size() > 1) {
            // Consider the next step in the path
            vec2 nextStep = path[1]; // Assuming path[0] is the current position
            vec2 direction = normalize(nextStep - motion.position);
            float speed = 40.0f; // Tune this value as needed

            // Update velocity towards the next step in the path
            motion.velocity = direction * speed;

            // If the AI is close to the next step, consider moving to the subsequent step
            // This prevents stopping at each path point and smoothens movement
            if (length(motion.position - nextStep) < speed * elapsed_ms / 1000.0f) {
                if (path.size() > 2) { // Check if there is a next step
                    nextStep = path[2]; // Move to the next step
                }
            }
        }
    }
    else {
        // If line of sight is not clear, stop the entity or handle accordingly
        motion.velocity = vec2(0.0f, 0.0f);
    }
}

void AISystem::handleRangedAI(Entity entity, Motion& motion, AI& ai, float elapsed_ms, const vec2& playerPosition) {
    float shootingRange = 500.0f; // Distance to shoot at the player
    float playerAvoidanceDistance = 150.0f; // Distance to keep from the player

    vec2 avoidanceForce(0.0f);
    float obstacleAvoidanceRadius = 50.0f; // radius within which to avoid obstacles
    float projectileAvoidanceRadius = 50.0f; // radius within which to avoid projectiles

    Level& level = registry.levels.get(registry.levels.entities[0]);
    Room& room = registry.rooms.get(level.rooms[level.current_room]);
   
    vec2 flockMove = flockMovement(entity, motion, elapsed_ms, playerPosition, playerAvoidanceDistance);

    // Calculate distance to the player
    float distanceToPlayer = length(playerPosition - motion.position);

    float maxSpeed = 50.0f; // Adjust as needed

    // Movement logic considering player's distance and projectiles
    if (distanceToPlayer > playerAvoidanceDistance) {
        // Move normally if within shooting range but outside avoidance distance
        motion.velocity = flockMove;
    }
    else if (distanceToPlayer <= playerAvoidanceDistance) {
        // Move away from the player if too close
        vec2 awayFromPlayer = normalize(motion.position - playerPosition) * 100.0f; // Strength of repulsion
        motion.velocity = flockMove + awayFromPlayer;
    }
    
    // Calculate the next position based on current velocity
    vec2 nextPosition = motion.position + (motion.velocity * elapsed_ms / 1000.0f);

    // kept value 100, 64 is too small and enemies get stuck
    // Check if the next position is within bounds
    if (!isPositionWithinBounds(nextPosition)) {
        // Adjust velocity to bounce off the wall
        if (nextPosition.x <= xMin + (cellSize + halfCellSize) || nextPosition.x >= xMax - (cellSize + halfCellSize)) {
            motion.velocity.x *= -1; // Invert X component if hitting left or right wall
        }
        if (nextPosition.y <= yMin + (cellSize + halfCellSize) || nextPosition.y >= yMax - (cellSize + halfCellSize)) {
            motion.velocity.y *= -1; // Invert Y component if hitting top or bottom wall
        }
    }

    // Avoidance behavior for projectiles
    for (auto& projectileEntity : registry.projectiles.entities) {
        // check if player projectile 
        if (registry.projectiles.get(projectileEntity).source == registry.players.entities[0]) {
            Motion& projectileMotion = registry.motions.get(projectileEntity);
            vec2 directionToProjectile = projectileMotion.position - motion.position;
            float distanceToProjectile = length(directionToProjectile);

            if (distanceToProjectile < projectileAvoidanceRadius) {
                avoidanceForce -= (directionToProjectile / distanceToProjectile);
            }
        }
        else {
            break; // No need to check further projectiles if not player's
        }
        
    }

    // Apply avoidance force to the current motion
    if (glm::length(avoidanceForce) > 0) {
        motion.velocity += normalize(avoidanceForce) * (100.0f * elapsed_ms);
    }

    // Clamp velocity to max speed
    if (length(motion.velocity) > maxSpeed) {
        motion.velocity = normalize(motion.velocity) * maxSpeed;
    }

    // Shooting logic
    if (distanceToPlayer <= shootingRange && lineOfSightClear(motion.position, playerPosition)) {
        vec2 direction = normalize(playerPosition - motion.position);
        motion.look_angle = atan2(direction.y, direction.x) - M_PI / 2;

        ai.shootingCooldown -= elapsed_ms / 1000.f; // Convert milliseconds to seconds
        if (ai.shootingCooldown <= 0) {
            vec2 shootingDirection = normalize(playerPosition - motion.position);
            float shootingAngle = atan2(shootingDirection.y, shootingDirection.x);
            createEnemySniperProjectile(renderer, motion.position, shootingAngle, entity);
            ai.shootingCooldown = 1.f; // Reset cooldown
        }
	}
}

void AISystem::handleShotgunAI(Entity entity, Motion& motion, AI& ai, float elapsed_ms, const vec2& playerPosition) {
    float shootingRange = 250.0f; // Distance to shoot at the player
    float playerAvoidanceDistance = 50.0f; // Distance to keep from the player

    vec2 avoidanceForce(0.0f);
    float obstacleAvoidanceRadius = 100.0f; // radius within which to avoid obstacles
    float projectileAvoidanceRadius = 250.0f; // radius within which to avoid projectiles

    Level& level = registry.levels.get(registry.levels.entities[0]);
    Room& room = registry.rooms.get(level.rooms[level.current_room]);

    vec2 flockMove = flockMovement(entity, motion, elapsed_ms, playerPosition, playerAvoidanceDistance);

    // Calculate distance to the player
    float distanceToPlayer = length(playerPosition - motion.position);

    float maxSpeed = 100.0f; // Adjust as needed

    // Movement logic considering player's distance and projectiles
    if (distanceToPlayer > playerAvoidanceDistance) {
        // Move normally if within shooting range but outside avoidance distance
        motion.velocity = flockMove;
    }
    else if (distanceToPlayer <= playerAvoidanceDistance) {
        // Move away from the player if too close
        vec2 awayFromPlayer = normalize(motion.position - playerPosition) * 100.0f; // Strength of repulsion
        motion.velocity = flockMove + awayFromPlayer;
    }

    // Calculate the next position based on current velocity
    vec2 nextPosition = motion.position + (motion.velocity * elapsed_ms / 1000.0f);

    // kept value 100, 64 is too small and enemies get stuck
    // Check if the next position is within bounds
    if (!isPositionWithinBounds(nextPosition)) {
        // Adjust velocity to bounce off the wall
        if (nextPosition.x <= xMin + (cellSize + halfCellSize) || nextPosition.x >= xMax - (cellSize + halfCellSize)) {
            motion.velocity.x *= -1; // Invert X component if hitting left or right wall
        }
        if (nextPosition.y <= yMin + (cellSize + halfCellSize) || nextPosition.y >= yMax - (cellSize + halfCellSize)) {
            motion.velocity.y *= -1; // Invert Y component if hitting top or bottom wall
        }
    }

    // Avoidance behavior for projectiles
    for (auto& projectileEntity : registry.projectiles.entities) {
        // check if player projectile 
        if (registry.projectiles.get(projectileEntity).source == registry.players.entities[0]) {
            Motion& projectileMotion = registry.motions.get(projectileEntity);
            vec2 directionToProjectile = projectileMotion.position - motion.position;
            float distanceToProjectile = length(directionToProjectile);

            if (distanceToProjectile < projectileAvoidanceRadius) {
                avoidanceForce -= (directionToProjectile / distanceToProjectile);
            }
        }
        else {
            break; // No need to check further projectiles if not player's
        }

    }

    // Apply avoidance force to the current motion
    if (glm::length(avoidanceForce) > 0) {
        motion.velocity += normalize(avoidanceForce) * (100.0f * elapsed_ms);
    }

    // Clamp velocity to max speed
    if (length(motion.velocity) > maxSpeed) {
        motion.velocity = normalize(motion.velocity) * maxSpeed;
    }

    // Shooting logic
    if (distanceToPlayer <= shootingRange && lineOfSightClear(motion.position, playerPosition)) {
        vec2 direction = normalize(playerPosition - motion.position);
        motion.look_angle = atan2(direction.y, direction.x) - M_PI/2;

        ai.shootingCooldown -= elapsed_ms / 1000.f; // Convert milliseconds to seconds
        if (ai.shootingCooldown <= 0) {
            vec2 shootingDirection = normalize(playerPosition - motion.position);
            float shootingAngle = atan2(shootingDirection.y, shootingDirection.x);
            for (int i = 0; i < 10; i++) {
                createShotgunProjectile(renderer, motion.position, shootingAngle, 0.0, 0.0, i, entity);
            }
            ai.shootingCooldown = 1.5f; // Reset cooldown
        }
    }
}

void AISystem::handleRocketAI(Entity entity, Motion& motion, AI& ai, float elapsed_ms, const vec2& playerPosition) {
    float shootingRange = 250.0f; // Distance to shoot at the player
    float playerAvoidanceDistance = 50.0f; // Distance to keep from the player

    vec2 avoidanceForce(0.0f);
    float obstacleAvoidanceRadius = 100.0f; // radius within which to avoid obstacles
    float projectileAvoidanceRadius = 250.0f; // radius within which to avoid projectiles

    Level& level = registry.levels.get(registry.levels.entities[0]);
    Room& room = registry.rooms.get(level.rooms[level.current_room]);

    vec2 flockMove = flockMovement(entity, motion, elapsed_ms, playerPosition, playerAvoidanceDistance);

    // Calculate distance to the player
    float distanceToPlayer = length(playerPosition - motion.position);

    float maxSpeed = 100.0f; // Adjust as needed

    // Movement logic considering player's distance and projectiles
    if (distanceToPlayer > playerAvoidanceDistance) {
        // Move normally if within shooting range but outside avoidance distance
        motion.velocity = flockMove;
    }
    else if (distanceToPlayer <= playerAvoidanceDistance) {
        // Move away from the player if too close
        vec2 awayFromPlayer = normalize(motion.position - playerPosition) * 100.0f; // Strength of repulsion
        motion.velocity = flockMove + awayFromPlayer;
    }

    // Calculate the next position based on current velocity
    vec2 nextPosition = motion.position + (motion.velocity * elapsed_ms / 1000.0f);

    // kept value 100, 64 is too small and enemies get stuck
    // Check if the next position is within bounds
    if (!isPositionWithinBounds(nextPosition)) {
        // Adjust velocity to bounce off the wall
        if (nextPosition.x <= xMin + (cellSize + halfCellSize) || nextPosition.x >= xMax - (cellSize + halfCellSize)) {
            motion.velocity.x *= -1; // Invert X component if hitting left or right wall
        }
        if (nextPosition.y <= yMin + (cellSize + halfCellSize) || nextPosition.y >= yMax - (cellSize + halfCellSize)) {
            motion.velocity.y *= -1; // Invert Y component if hitting top or bottom wall
        }
    }

    // Avoidance behavior for projectiles
    for (auto& projectileEntity : registry.projectiles.entities) {
        // check if player projectile 
        if (registry.projectiles.get(projectileEntity).source == registry.players.entities[0]) {
            Motion& projectileMotion = registry.motions.get(projectileEntity);
            vec2 directionToProjectile = projectileMotion.position - motion.position;
            float distanceToProjectile = length(directionToProjectile);

            if (distanceToProjectile < projectileAvoidanceRadius) {
                avoidanceForce -= (directionToProjectile / distanceToProjectile);
            }
        }
        else {
            break; // No need to check further projectiles if not player's
        }

    }

    // Apply avoidance force to the current motion
    if (glm::length(avoidanceForce) > 0) {
        motion.velocity += normalize(avoidanceForce) * (100.0f * elapsed_ms);
    }

    // Clamp velocity to max speed
    if (length(motion.velocity) > maxSpeed) {
        motion.velocity = normalize(motion.velocity) * maxSpeed;
    }

    // Shooting logic
    if (distanceToPlayer <= shootingRange && lineOfSightClear(motion.position, playerPosition)) {
        vec2 direction = normalize(playerPosition - motion.position);
        motion.look_angle = atan2(direction.y, direction.x) - M_PI / 2;

        ai.shootingCooldown -= elapsed_ms / 1000.f; // Convert milliseconds to seconds
        if (ai.shootingCooldown <= 0) {
            vec2 shootingDirection = normalize(playerPosition - motion.position);
            float shootingAngle = atan2(shootingDirection.y, shootingDirection.x);
            for (int i = 0; i < 5; i++) {
                createEnemyRocketProjectile(renderer, motion.position, shootingAngle, entity);
            }
            ai.shootingCooldown = 1.5f; // Reset cooldown
        }
    }
}

void AISystem::handleFlameAI(Entity entity, Motion& motion, AI& ai, float elapsed_ms, const vec2& playerPosition) {
    float shootingRange = 250.0f; // Distance to shoot at the player
    float playerAvoidanceDistance = 50.0f; // Distance to keep from the player

    vec2 avoidanceForce(0.0f);
    float obstacleAvoidanceRadius = 100.0f; // radius within which to avoid obstacles
    float projectileAvoidanceRadius = 250.0f; // radius within which to avoid projectiles

    Level& level = registry.levels.get(registry.levels.entities[0]);
    Room& room = registry.rooms.get(level.rooms[level.current_room]);

    vec2 flockMove = flockMovement(entity, motion, elapsed_ms, playerPosition, playerAvoidanceDistance);

    // Calculate distance to the player
    float distanceToPlayer = length(playerPosition - motion.position);

    float maxSpeed = 100.0f; // Adjust as needed

    // Movement logic considering player's distance and projectiles
    if (distanceToPlayer > playerAvoidanceDistance) {
        // Move normally if within shooting range but outside avoidance distance
        motion.velocity = flockMove;
    }
    else if (distanceToPlayer <= playerAvoidanceDistance) {
        // Move away from the player if too close
        vec2 awayFromPlayer = normalize(motion.position - playerPosition) * 100.0f; // Strength of repulsion
        motion.velocity = flockMove + awayFromPlayer;
    }

    // Calculate the next position based on current velocity
    vec2 nextPosition = motion.position + (motion.velocity * elapsed_ms / 1000.0f);

    // kept value 100, 64 is too small and enemies get stuck
    // Check if the next position is within bounds
    if (!isPositionWithinBounds(nextPosition)) {
        // Adjust velocity to bounce off the wall
        if (nextPosition.x <= xMin + (cellSize + halfCellSize) || nextPosition.x >= xMax - (cellSize + halfCellSize)) {
            motion.velocity.x *= -1; // Invert X component if hitting left or right wall
        }
        if (nextPosition.y <= yMin + (cellSize + halfCellSize) || nextPosition.y >= yMax - (cellSize + halfCellSize)) {
            motion.velocity.y *= -1; // Invert Y component if hitting top or bottom wall
        }
    }

    // Avoidance behavior for projectiles
    for (auto& projectileEntity : registry.projectiles.entities) {
        // check if player projectile 
        if (registry.projectiles.get(projectileEntity).source == registry.players.entities[0]) {
            Motion& projectileMotion = registry.motions.get(projectileEntity);
            vec2 directionToProjectile = projectileMotion.position - motion.position;
            float distanceToProjectile = length(directionToProjectile);

            if (distanceToProjectile < projectileAvoidanceRadius) {
                avoidanceForce -= (directionToProjectile / distanceToProjectile);
            }
        }
        else {
            break; // No need to check further projectiles if not player's
        }

    }

    // Apply avoidance force to the current motion
    if (glm::length(avoidanceForce) > 0) {
        motion.velocity += normalize(avoidanceForce) * (100.0f * elapsed_ms);
    }

    // Clamp velocity to max speed
    if (length(motion.velocity) > maxSpeed) {
        motion.velocity = normalize(motion.velocity) * maxSpeed;
    }

    // Shooting logic
    if (distanceToPlayer <= shootingRange && lineOfSightClear(motion.position, playerPosition)) {
        vec2 direction = normalize(playerPosition - motion.position);
        motion.look_angle = atan2(direction.y, direction.x) - M_PI / 2;

        ai.shootingCooldown -= elapsed_ms / 1000.f; // Convert milliseconds to seconds
        if (ai.shootingCooldown <= 0) {
            vec2 shootingDirection = normalize(playerPosition - motion.position);
            float shootingAngle = atan2(shootingDirection.y, shootingDirection.x);
            createEnemyFlamethrowerProjectile(renderer, motion.position, shootingAngle, entity);
            ai.shootingCooldown = 0.05f; // Reset cooldown
        }
    }
}

void AISystem::handleTurretAI(Entity entity, Motion& motion, AI& ai, float elapsed_ms, const vec2& playerPosition) {
    // Check for line of sight to the player
    if (lineOfSightClear(motion.position, playerPosition)) {
        // Rotate turret to face player - Calculate the angle between the turret and the player
        vec2 direction = normalize(playerPosition - motion.position);
        motion.look_angle = atan2(direction.y, direction.x) + M_PI / 2;

        // Shooting logic, no range for these long ranged turrets
        ai.shootingCooldown -= elapsed_ms / 1000.f; // Cooldown reduction
        if (ai.shootingCooldown <= 0) {
            vec2 shootingDirection = normalize(playerPosition - motion.position);
            float shootingAngle = atan2(shootingDirection.y, shootingDirection.x);
            createEnemyProjectile(renderer, motion.position, shootingAngle, entity);
            ai.shootingCooldown = .5f; // Reset cooldown
        }
    }
}

bool AISystem::isPositionWithinBounds(const vec2& position) {
    // kept value 100, 64 is too small and enemies get stuck
    // Using the game world boundaries 
    float leftBound = xMin + (cellSize + halfCellSize);
    float rightBound = xMax - (cellSize + halfCellSize);
    float topBound = yMin + (cellSize + halfCellSize);
    float bottomBound = yMax - (cellSize + halfCellSize);

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
