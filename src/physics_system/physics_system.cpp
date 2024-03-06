// internal
#include "physics_system/physics_system.hpp"
#include "world_init/world_init.hpp"

// TODO: Improve bounding box collision detection into a more accurate one

// Returns the local bounding coordinates scaled by the current size of the entity
vec2 get_bounding_box(const Motion &motion)
{
	// abs is to avoid negative scale due to the facing direction, account for rotation i.e motion.look_angle
	float x_scale = abs(motion.scale.x) * cos(motion.look_angle) + abs(motion.scale.y) * sin(motion.look_angle);
	float y_scale = abs(motion.scale.x) * sin(motion.look_angle) + abs(motion.scale.y) * cos(motion.look_angle);

	return {x_scale, y_scale};
}


// Material for the SAT collision detection is sourced from the following blog post
// https://code.tutsplus.com/collision-detection-using-the-separating-axis-theorem--gamedev-169t

// Narrow check
bool sat_collision_check(Entity e1, Entity e2) {
	Motion& m1 = registry.motions.get(e1);
	Motion& m2 = registry.motions.get(e2);

	std::vector<vec2> e1_pts;
	std::vector<vec2> e2_pts;

	// Get all vertices of the convex hull for entity1
	if (registry.meshPtrs.has(e1)) {
		std::vector<ColoredVertex>& vertices = registry.meshPtrs.get(e1)->vertices;
		Transform t;
		t.translate(m1.position);
		t.rotate(m1.look_angle);
		t.scale(m1.scale);

		for (int i = 0; i < vertices.size(); i++) {
			vec3 vert = t.mat * vec3({ vertices[i].position.x, vertices[i].position.y, 1.0f });
			e1_pts.push_back({vert.x, vert.y});
		}

	} else {
		e1_pts.push_back(vec2({ m1.position.x - m1.scale.x / 2, m1.position.y - m1.scale.y / 2 })); // top-left
		e1_pts.push_back(vec2({ m1.position.x + m1.scale.x / 2, m1.position.y - m1.scale.y / 2 })); // top-right
		e1_pts.push_back(vec2({ m1.position.x + m1.scale.x / 2, m1.position.y + m1.scale.y / 2 })); // bottom-right
		e1_pts.push_back(vec2({ m1.position.x - m1.scale.x / 2, m1.position.y + m1.scale.y / 2 })); // bottom-left
	}

	// Get vertices of the convex hull for entity2
	if (registry.meshPtrs.has(e2)) {
		std::vector<ColoredVertex>& vertices = registry.meshPtrs.get(e2)->vertices;
		Transform t;
		t.translate(m2.position);
		t.rotate(m2.look_angle);
		t.scale(m2.scale);

		for (int i = 0; i < vertices.size(); i++) {
			vec3 vert = t.mat * vec3({ vertices[i].position.x, vertices[i].position.y, 1.0f });
			e2_pts.push_back({ vert.x, vert.y });
		}

	} else {
		e2_pts.push_back(vec2({ m2.position.x - m2.scale.x / 2, m2.position.y - m2.scale.y / 2 })); // top-left
		e2_pts.push_back(vec2({ m2.position.x + m2.scale.x / 2, m2.position.y + m2.scale.y / 2 })); // top-right
		e2_pts.push_back(vec2({ m2.position.x + m2.scale.x / 2, m2.position.y + m2.scale.y / 2 })); // bottom-right
		e2_pts.push_back(vec2({ m2.position.x - m2.scale.x / 2, m2.position.y - m2.scale.y / 2 })); // bottom-left
	}

	std::vector<vec2> e1_normals;
	std::vector<vec2> e2_normals;

	// Get the left-normals of each face of the convex hull (since the vertices are in clockwise order)
	for (int i = e1_pts.size() - 1; i >= 0; i--) {
		vec2& p1 = e1_pts[i];
		vec2& p2 = e1_pts[(i + 1) % e1_pts.size()];
		
		e1_normals.push_back(vec2({ -(p2.y - p1.y), p2.x - p1.x }));
	}

	for (int i = e2_pts.size() - 1; i >= 0; i--) {
		vec2& p1 = e2_pts[i];
		vec2& p2 = e2_pts[(i + 1) % e2_pts.size()];

		e2_normals.push_back(vec2({ -(p2.y - p1.y), p2.x - p1.x }));
	}

	float min_e1;
	float max_e1;
	float min_e2;
	float max_e2;

	// Check for SAT collision with normals of entity1
	for (vec2& n : e1_normals) {
		min_e1 = glm::dot(e1_pts[0], n);
		max_e1 = min_e1;
		min_e2 = glm::dot(e2_pts[0], n);
		max_e2 = min_e2;

		for (int i = 1; i < e1_pts.size(); i++) {
			float dot_prod = glm::dot(e1_pts[i], n);
			if (dot_prod < min_e1) {
				min_e1 = dot_prod;
			} else if (dot_prod > max_e1) {
				max_e1 = dot_prod;
			}
		}

		for (int i = 1; i < e2_pts.size(); i++) {
			float dot_prod = glm::dot(e2_pts[i], n);
			if (dot_prod < min_e2) {
				min_e2 = dot_prod;
			} else if (dot_prod > max_e2) {
				max_e2 = dot_prod;
			}
		}

		// We can draw a line between the two entities, return no collision
		if (min_e1 > max_e2 || min_e2 > max_e1) {
			return false;
		}
	}

	// Check for SAT collision with normals from entity2
	for (vec2& n : e2_normals) {
		min_e1 = glm::dot(e1_pts[0], n);
		max_e1 = min_e1;
		min_e2 = glm::dot(e2_pts[0], n);
		max_e2 = min_e2;

		for (int i = 1; i < e1_pts.size(); i++) {
			float dot_prod = glm::dot(e1_pts[i], n);
			if (dot_prod < min_e1) {
				min_e1 = dot_prod;
			} else if (dot_prod > max_e1) {
				max_e1 = dot_prod;
			}
		}

		for (int i = 1; i < e2_pts.size(); i++) {
			float dot_prod = glm::dot(e2_pts[i], n);
			if (dot_prod < min_e2) {
				min_e2 = dot_prod;
			} else if (dot_prod > max_e2) {
				max_e2 = dot_prod;
			}
		}

		// We can draw a line between the two entities, return no collision
		if (min_e1 > max_e2 || min_e2 > max_e1) {
			return false;
		}
	}

	return true;
 }

// Broad check
 bool aabb_collision_check(const Motion& motion1, const Motion& motion2) {
	 const float& x1 = motion1.position.x;
	 const float& y1 = motion1.position.y;
	 const float& w1 = abs(motion1.scale.x);
	 const float& h1 = abs(motion1.scale.y);

	 const float& x2 = motion2.position.x;
	 const float& y2 = motion2.position.y;
	 const float& w2 = abs(motion2.scale.x);
	 const float& h2 = abs(motion2.scale.y);
	 

	 return
		 (x1 - 0.5f * w1 < x2 + 0.5f * w2 && y1 - 0.5f * h1 < y2 + 0.5 * h2) &&
		 (x2 - 0.5f * w2 < x1 + 0.5f * w1 && y2 - 0.5f * h2 < y1 + 0.5f * h1);
}

// Returns whether two entities collide based on AABB collision detection
bool collides(const Entity entity1, const Entity entity2)
{
	Motion& m1 = registry.motions.get(entity1);
	Motion& m2 = registry.motions.get(entity2);

	if (!aabb_collision_check(m1, m2)) {
		return false;
	}

	return sat_collision_check(entity1, entity2);
}

void update_motion(Motion &motion, float step_seconds)
{
	// TODO: Update position with motion here
	if (!motion.complex)
	{
		motion.position.x += motion.velocity.x * step_seconds;
		motion.position.y += motion.velocity.y * step_seconds;

		return;
	}

	float potential_velocity_delta_x = 0.f;
	float potential_velocity_delta_y = 0.f;

	// TODO: Make acceleration and deceleration work as variables on current velocty

	float velocity_magnitude = sqrt(motion.velocity.x * motion.velocity.x + motion.velocity.y * motion.velocity.y);

	// UP - DOWN CASES (these cancel each other out)
	if (motion.is_moving_up && !motion.is_moving_down)
	{
		potential_velocity_delta_y -= motion.acceleration_rate;
	}
	else if (motion.is_moving_down && !motion.is_moving_up)
	{
		potential_velocity_delta_y += motion.acceleration_rate;
	}
	else
	{
		// decelerate case
		if ((motion.velocity.y - motion.deceleration_rate) >= 0.f)
		{
			potential_velocity_delta_y -= motion.deceleration_rate;
		}
		else if (motion.velocity.y <= -motion.deceleration_rate)
		{
			potential_velocity_delta_y += motion.deceleration_rate;
		}
		else
		{
			potential_velocity_delta_y = -motion.velocity.y;
		}
	}

	// LEFT - RIGHT CASES (these cancel each other out)
	if (motion.is_moving_left && !motion.is_moving_right)
	{
		potential_velocity_delta_x -= motion.acceleration_rate;
	}
	else if (motion.is_moving_right && !motion.is_moving_left)
	{
		potential_velocity_delta_x += motion.acceleration_rate;
	}
	else
	{
		// decelerate case
		if ((motion.velocity.x - motion.deceleration_rate) >= 0.f)
		{
			potential_velocity_delta_x -= motion.deceleration_rate;
		}
		else if (motion.velocity.x <= -motion.deceleration_rate)
		{
			potential_velocity_delta_x += motion.deceleration_rate;
		}
		else
		{
			// needed as we can still be
			potential_velocity_delta_x = -motion.velocity.x;
		}
	}
	
	// Update velocity
	motion.velocity.x += potential_velocity_delta_x;
	motion.velocity.y += potential_velocity_delta_y;

	// normalize velocity to max velocity if needed
	velocity_magnitude = sqrt(motion.velocity.x * motion.velocity.x + motion.velocity.y * motion.velocity.y);
	if (velocity_magnitude > motion.max_velocity)
	{
		motion.velocity.x = motion.velocity.x / velocity_magnitude * motion.max_velocity;
		motion.velocity.y = motion.velocity.y / velocity_magnitude * motion.max_velocity;
	}

	// Update position
	motion.position.x += motion.velocity.x * step_seconds;
	motion.position.y += motion.velocity.y * step_seconds;

	// TODO: clamp position to game window, through collision logic
	float max_position = (game_window_size_px / 2) - (game_window_block_size / 2);
	motion.position.x = clamp(motion.position.x, (window_width_px / 2) - max_position, (window_width_px / 2) + max_position);
	motion.position.y = clamp(motion.position.y, (window_height_px / 2) - max_position, (window_height_px / 2) + max_position);
}

void PhysicsSystem::step(float elapsed_ms)
{
	// Move entity with motion based on how much time has passed, this is to (partially) avoid
	// having entities move at different speed based on the machine.
	auto &motion_registry = registry.motions;
	for (uint i = 0; i < motion_registry.size(); i++)
	{

		Motion &motion = motion_registry.components[i];
		// Entity entity = motion_registry.entities[i];
		float step_seconds = elapsed_ms / 1000.f;
		// update the motion of the entity
		update_motion(motion, step_seconds);
	}

	// Check for collisions between all moving entities
	ComponentContainer<Motion> &motion_container = registry.motions;
	for (uint i = 0; i < motion_container.components.size(); i++)
	{
		Motion &motion_i = motion_container.components[i];
		Entity entity_i = motion_container.entities[i];

		if (registry.noCollisionChecks.has(entity_i)) {
			continue;
		}

		// note starting j at i+1 to compare all (i,j) pairs only once (and to not compare with itself)
		for (uint j = i + 1; j < motion_container.components.size(); j++)
		{
			Entity entity_j = motion_container.entities[j];	

			if (registry.noCollisionChecks.has(entity_j)) {
				continue;
			}

			if (collides(entity_i, entity_j))
			{
				// Create a collisions event
				// We are abusing the ECS system a bit in that we potentially insert muliple collisions for the same entity
				registry.collisions.emplace_with_duplicates(entity_i, entity_j);
				registry.collisions.emplace_with_duplicates(entity_j, entity_i);
			}
		}
	}
}