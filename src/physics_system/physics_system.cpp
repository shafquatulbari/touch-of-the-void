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

// Returns whether two entities collide based on AABB collision detection
bool collides(const Motion &motion1, const Motion &motion2)
{
	const float& x1 = motion1.position.x;
	const float& y1 = motion1.position.y;
	const float& w1 = abs(abs(motion1.scale.x) * cos(motion1.look_angle) + abs(motion1.scale.y) * sin(motion1.look_angle));
	const float& h1 = abs(abs(motion1.scale.x) * sin(motion1.look_angle) + abs(motion1.scale.y) * cos(motion1.look_angle));

	const float& x2 = motion2.position.x;
	const float& y2 = motion2.position.y;
	const float& w2 = abs(abs(motion2.scale.x) * cos(motion2.look_angle) + abs(motion2.scale.y) * sin(motion2.look_angle));
	const float& h2 = abs(abs(motion2.scale.x) * sin(motion2.look_angle) + abs(motion2.scale.y) * cos(motion2.look_angle));

	return
		(x1 - 0.5f*w1 < x2 + 0.5f*w2 && y1 - 0.5f*h1 < y2 + 0.5*h2) &&
		(x2 - 0.5f*w2 < x1 + 0.5f*w1 && y2 - 0.5f*h2 < y1 + 0.5f*h1);
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

		// note starting j at i+1 to compare all (i,j) pairs only once (and to not compare with itself)
		for (uint j = i + 1; j < motion_container.components.size(); j++)
		{
			Motion &motion_j = motion_container.components[j];
			if (collides(motion_i, motion_j))
			{
				Entity entity_j = motion_container.entities[j];
				// Create a collisions event
				// We are abusing the ECS system a bit in that we potentially insert muliple collisions for the same entity
				registry.collisions.emplace_with_duplicates(entity_i, entity_j);
				registry.collisions.emplace_with_duplicates(entity_j, entity_i);
			}
		}
	}
}