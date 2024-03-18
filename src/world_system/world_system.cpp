// Header
#include "world_system/world_system.hpp"
#include "world_init/world_init.hpp"
#include "physics_system/physics_system.hpp"
#include "ui_system/ui_system.hpp"
#include <weapon_system/weapon_system.hpp>

// stlib
#include <cassert>
#include <sstream>
#include <iostream>
#include <glm/trigonometric.hpp>

// Game configuration
// TODO: set hard coded game configuration values here

// Create the world
WorldSystem::WorldSystem()
{
	// TODO: world initialization here
	
	//max fps 
	maxFps = 144.0f;
	// Seeding rng with random device
	rng = std::default_random_engine(std::random_device()());
}

// Destroy the world
WorldSystem::~WorldSystem()
{
	// Destroy sound components
	close_audio();

	// Destroy all created components
	registry.clear_all_components();

	// Destroy the window
	glfwDestroyWindow(window);
}

// Debugging
namespace {
	void glfw_err_cb(int error, const char* desc) {
		fprintf(stderr, "%d: %s", error, desc);
	}
}

// World initialization
// Note, this has a lot of OpenGL specific things, could be moved to the renderer
GLFWwindow* WorldSystem::create_window() {
	///////////////////////////////////////
	// Initialize GLFW
	glfwSetErrorCallback(glfw_err_cb);
	if (!glfwInit()) {
		fprintf(stderr, "Failed to initialize GLFW");
		return nullptr;
	}

	//-------------------------------------------------------------------------
	// If you are on Linux or Windows, you can change these 2 numbers to 4 and 3 and
	// enable the glDebugMessageCallback to have OpenGL catch your mistakes for you.
	// GLFW / OGL Initialization
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
	#if __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	#endif
	glfwWindowHint(GLFW_RESIZABLE, 0);

	// Create the main window (for rendering, keyboard, and mouse input)
	window = glfwCreateWindow(window_width_px, window_height_px, "Touch of the Void", nullptr, nullptr);
	if (window == nullptr) {
		fprintf(stderr, "Failed to glfwCreateWindow");
		return nullptr;
	}

	// Setting callbacks to member functions (that's why the redirect is needed)
	// Input is handled using GLFW, for more info see
	// http://www.glfw.org/docs/latest/input_guide.html
	glfwSetWindowUserPointer(window, this);
	auto key_redirect = [](GLFWwindow* wnd, int _0, int _1, int _2, int _3) { ((WorldSystem*)glfwGetWindowUserPointer(wnd))->on_key(_0, _1, _2, _3); };
	auto cursor_pos_redirect = [](GLFWwindow* wnd, double _0, double _1) { ((WorldSystem*)glfwGetWindowUserPointer(wnd))->on_mouse_move({ _0, _1 }); };
	auto mouse_button_redirect = [](GLFWwindow* wnd, int _0, int _1, int _2) { ((WorldSystem*)glfwGetWindowUserPointer(wnd))->on_mouse_click(_0, _1, _2); };
	glfwSetKeyCallback(window, key_redirect);
	glfwSetCursorPosCallback(window, cursor_pos_redirect);
	glfwSetMouseButtonCallback(window, mouse_button_redirect);
	
	//////////////////////////////////////
	// Loading music and sounds with SDL
	if (!init_audio()) {
		fprintf(stderr, "Failed to initialize audio");
		return nullptr;
	}

	return window;
}

void WorldSystem::init(RenderSystem* renderer_arg, UISystem* ui_arg, WeaponSystem* weapon_arg) {
	this->renderer = renderer_arg;
	this->ui = ui_arg;
	this->weapons = weapon_arg;

	//std::stringstream title_ss;
	//title_ss << "Touch of the Void";
	//glfwSetWindowTitle(window, title_ss.str().c_str());


	// TODO: Setup background music to play indefinitely
	//Mix_PlayMusic(background_music, -1);
	//fprintf(stderr, "Loaded music\n");

	// Set all states to default
	restart_game();
}

// Update our game world
bool WorldSystem::step(float elapsed_ms_since_last_update) 
{
	switch (game_state)
	{
	case GAME_STATE::START_MENU:
		return true;
		break;
	case GAME_STATE::GAME:
		break;

	case GAME_STATE::GAME_OVER:
		return true;
		break;

	default:
		return true;
		break;
	}

	// Remove debug info from the last step
	while (registry.debugComponents.entities.size() > 0)
		registry.remove_all_components_of(registry.debugComponents.entities.back());


	auto& motions_registry = registry.motions;
    
	Player& p = registry.players.get(player);
	Motion& p_m = registry.motions.get(player);

	// Update HUD
	ui->update(registry.healths.get(player), registry.shields.get(player), registry.players.get(player), score, multiplier, 0);

	// Update Weapon System
	weapons->step(elapsed_ms_since_last_update, renderer, player);

	// Removing out of screen entities
	// Remove entities that leave the screen on the left side
	// Iterate backwards to be able to remove without unterfering with the next object to visit
	// (the containers exchange the last element with the current)
	for (int i = (int)motions_registry.components.size() - 1; i >= 0; --i) {
		Motion& motion = motions_registry.components[i];
		Entity& entity = motions_registry.entities[i];
		if (motion.position.x + abs(motion.scale.x) < 0.f) {
			if (!registry.players.has(entity)) // don't remove the player
				registry.remove_all_components_of(entity);
		}

		if (registry.projectiles.has(entity)) {
			// max_position from physics_system.cpp and replaced game_window_block_size 
			// with the entity's width and height
			float max_position_x = (game_window_size_px / 2) - (motion.scale.x / 2);
			float max_position_y = (game_window_size_px / 2) - (motion.scale.y / 2);
			if (
				abs(motion.position.x - (window_width_px / 2)) >= max_position_x ||
				abs(motion.position.y - (window_height_px / 2)) >= max_position_y
			)
					registry.remove_all_components_of(entity);
		}
	}

	ScreenState& screen = registry.screenStates.components[0];

	float min_counter_ms = 3000.f;
	for (Entity entity : registry.deathTimers.entities) {
		// progress timer
		DeathTimer& counter = registry.deathTimers.get(entity);
		counter.counter_ms -= elapsed_ms_since_last_update;
		if (counter.counter_ms < min_counter_ms) {
			min_counter_ms = counter.counter_ms;
		}

		// restart the game once the death timer expired
		if (counter.counter_ms < 0) {
			registry.deathTimers.remove(entity);
			screen.darken_screen_factor = 0;
			game_state = GAME_STATE::GAME_OVER;
			restart_game();
			return true;
		}
	}
	for (Entity entity : registry.roomTransitionTimers.entities) {
		// progress timer
		RoomTransitionTimer& counter = registry.roomTransitionTimers.get(entity);
		counter.counter_ms -= elapsed_ms_since_last_update;
		if (counter.counter_ms < min_counter_ms) {
			min_counter_ms = counter.counter_ms;
		}

		// remove the darken effect once the timer expired
		if (counter.counter_ms < 0) {
			registry.roomTransitionTimers.remove(entity);
			screen.darken_screen_factor = 0;
			p.is_moving_rooms = false;
			// player enters new room
			return true;
		}
	}
	// reduce window brightness if any of the player is dying
	screen.darken_screen_factor = 1 - min_counter_ms / 3000;

	for (Entity entity : registry.animationTimers.entities) {
		// progress timer
		AnimationTimer& counter = registry.animationTimers.get(entity);
		if (!registry.animations.has(entity)) {
			registry.animationTimers.remove(entity);
			continue;
		}
		Animation& animation = registry.animations.get(entity);
		counter.counter_ms -= elapsed_ms_since_last_update;

		if (counter.counter_ms < 0) {
			registry.animationTimers.remove(entity);
			// check if finished
			if (animation.current_frame == animation.total_frames - 1) {
				if (animation.loop) {
					// restart animation
					animation.current_frame = 0;
					AnimationTimer& timer = registry.animationTimers.emplace(entity);
					timer.counter_ms = animation.frame_durations_ms[0];
				}
				else {
					// done animating
					registry.remove_all_components_of(entity);
				}
			}
			else {
				// progress to next frame
				animation.current_frame++;
				registry.animationTimers.remove(entity);
				AnimationTimer& timer = registry.animationTimers.emplace(entity);
				timer.counter_ms = animation.frame_durations_ms[animation.current_frame];
			}
		}
	}

	// heal obstacles over time
	for (auto& obstacle : registry.obstacles.entities) {
        if (registry.healths.has(obstacle)) {
			Health& health = registry.healths.get(obstacle);
			if (health.current_health < health.max_health) {
				health.current_health += (elapsed_ms_since_last_update / 1000.0f) * 50.0f; // Adjust speed here
				health.current_health = std::min(health.current_health, health.max_health); // don't exceed max health
			}
        }
    }

	// heal player over timne
	if (registry.healths.has(player)) {
		Health& playerHealth = registry.healths.get(player);
		if (playerHealth.current_health < playerHealth.max_health) {
			playerHealth.current_health += (elapsed_ms_since_last_update / 1000.0f) * 0.5; // Adjust speed here
			playerHealth.current_health = std::min(playerHealth.current_health, playerHealth.max_health); // don't exceed max health
		}

	}

	// FPS
	float startTicks = SDL_GetTicks();
	fpsCalculate();
	static int frameCounter = 0;
	frameCounter++;
	// update fps every 50 frames 
	if (frameCounter == 25) {
		if (debugging.show_fps)
			registry.texts.get(fps_text).content = "FPS: " + std::to_string(static_cast<int>(fps));
		frameCounter = 0;
	}
	float frameTicks = SDL_GetTicks() - startTicks;

	//fps limiter. 
	/*if ((1000.0f / maxFps) > frameTicks) {
		SDL_Delay((1000.0f / maxFps) - frameTicks);
	}*/

	////////////////////////////////////////////////////////////////////////////////////
	// Render mesh lines
	Mesh& p_mesh = *(registry.meshPtrs.get(player));
	Motion& p_motion = registry.motions.get(player);

	Transform t;
	t.translate(p_motion.position);
	t.rotate(p_motion.look_angle);
	t.scale(p_motion.scale);

	for (int i = 0; i < p_mesh.vertices.size(); i++) {
		Entity e = createLine(
			p_mesh.vertices[i],
			p_mesh.vertices[(i + 1) % p_mesh.vertices.size()],
			t.mat,
			{ 1.f, 0.f, 0.f },
			5
		);

		p_mesh_lines[i] = e;
	}

	////////////////////////////////////////////////////////////////////////////////////

	return true;
}

// Reset the world state to its initial state
void WorldSystem::restart_game() {
	// Debugging for memory/component leaks
	registry.list_all_components();
	printf("Restarting\n");

	// Reset darken_screen_factor on game restart
    ScreenState& screen = registry.screenStates.components[0];
    screen.darken_screen_factor = 0.0f; 

	// Remove all entities that we created
	// All that have a motion, we could also iterate over all bug, eagles, ... but that would be more cumbersome
	while (registry.motions.entities.size() > 0)
		registry.remove_all_components_of(registry.motions.entities.back());

	// Debugging for memory/component leaks
	registry.list_all_components();
	switch (game_state) 
	{
	case GAME_STATE::START_MENU:
		createText(renderer, "TOUCH OF THE VOID", { 960.0f, 324.0f }, 3.f, COLOR_RED, TextAlignment::CENTER);
		createText(renderer, "Press 'enter' to start", { 960.0f, 464.0f }, 1.f, COLOR_WHITE, TextAlignment::CENTER);
		break;

	case GAME_STATE::GAME:
		// Create a new player
		player = createPlayer(renderer, { window_width_px / 2, window_height_px / 2 });

		// Create a level
		registry.players.get(player).current_room = createBackground(renderer);

		//// Tutorial Text
		createText(renderer, "CONTROLS", { 38.0f, 144.0f }, 1.4f, COLOR_WHITE, TextAlignment::LEFT);
		createText(renderer, "WASD to move", { 30.0f, 224.0f }, 0.7f, COLOR_WHITE, TextAlignment::LEFT);
		createText(renderer, "Mouse to aim", { 30.0f, 264.0f }, 0.7f, COLOR_WHITE, TextAlignment::LEFT);
		createText(renderer, "Right-Click to shoot", { 30.0f, 304.0f }, 0.7f, COLOR_WHITE, TextAlignment::LEFT);
		createText(renderer, "R to reload", { 30.0f, 344.0f }, 0.7f, COLOR_WHITE, TextAlignment::LEFT);
		createText(renderer, "Q/E to change weapons", { 30.0f, 384.0f }, 0.7f, COLOR_WHITE, TextAlignment::LEFT);

		//// Create HUD
		score = 100;
		multiplier = 1.0;
		ui->init(renderer, registry.healths.get(player), registry.shields.get(player), registry.players.get(player), score, multiplier);
		break;

	case GAME_STATE::GAME_OVER:
		createText(renderer, "GAME OVER", { 960.0f, 324.0f }, 3.f, COLOR_RED, TextAlignment::CENTER);
		createText(renderer, "Press 'enter' to play again", { 960.0f, 464.0f }, 1.f, COLOR_WHITE, TextAlignment::CENTER);
		break;

	default:
		break;
	}
}


// Reset the world state to its initial state
void WorldSystem::enter_room(Room& room, vec2 player_pos) {
	// Debugging for memory/component leaks
	registry.list_all_components();
	printf("Entering Room\n");

	// Reset darken_screen_factor on room enter
	ScreenState& screen = registry.screenStates.components[0];
	screen.darken_screen_factor = 0.0f;

	for(Entity e: registry.motions.entities)
	{
		// clear motion registry except for player and texts.
		if (!(registry.players.has(e) || registry.texts.has(e)))
		{
			registry.remove_all_components_of(e);
		}
	}

	// Debugging for memory/component leaks
	registry.list_all_components();

	// Render the room
	render_room(renderer, room);
	ui->reinit();

	// Move the player to position
	registry.motions.get(player).position = player_pos;
}

// Compute collisions between entities
void WorldSystem::handle_collisions() {
	// Loop over all collisions detected by the physics system
	auto& collisionsRegistry = registry.collisions;
	for (uint i = 0; i < collisionsRegistry.components.size(); i++) {
		// The entity and its collider
		Entity entity = collisionsRegistry.entities[i];
		Entity entity_other = collisionsRegistry.components[i].other;
		float scalar = collisionsRegistry.components[i].scalar;
		
		if (registry.players.has(entity) && registry.obstacles.has(entity_other)) {

			for (Entity e : p_mesh_lines) {
				registry.lines.get(e).from.color = { 0.f, 1.f, 0.f };
				registry.lines.get(e).to.color = { 0.f, 1.f, 0.f };
			}

			Obstacle& obstacle = registry.obstacles.get(entity_other);
			if (obstacle.is_passable)
			{
				Player& player = registry.players.get(entity);
				if (!player.is_moving_rooms)
				{
					player.is_moving_rooms = true;
					vec2 next_pos;
					float x_mid = window_width_px / 2;
					float y_mid = window_height_px / 2;
					float x_delta = game_window_size_px / 2 - 16;
					float y_delta = game_window_size_px / 2 - 16;
					float x_max = x_mid + x_delta;
					float x_min = x_mid - x_delta;
					float y_max = y_mid + y_delta;
					float y_min = y_mid - y_delta;
					// change to the room that they entered
					if (obstacle.is_bottom_door)
					{
						player.current_room = registry.rooms.get(player.current_room).bottom_room;
						next_pos = { x_mid, y_min + 32 };
					}
					else if (obstacle.is_top_door)
					{
						player.current_room = registry.rooms.get(player.current_room).top_room;
						next_pos = { x_mid, y_max - 32 };
					}
					else if (obstacle.is_left_door)
					{
						player.current_room = registry.rooms.get(player.current_room).left_room;
						next_pos = { x_max - 32, y_mid };
					}
					else if (obstacle.is_right_door)
					{
						player.current_room = registry.rooms.get(player.current_room).right_room;
						next_pos = { x_min + 32, y_mid };
					}
				
					// darken effect
					registry.roomTransitionTimers.emplace(entity);
					enter_room(registry.rooms.get(player.current_room)
						, next_pos);
				}
			}

			// Apply damage to the player
			else if (registry.deadlies.has(entity_other)) {
				Health& playerHealth = registry.healths.get(player);
				Deadly& deadly = registry.deadlies.get(entity_other);
				playerHealth.current_health -= deadly.damage;
				if (playerHealth.current_health <= 0) {
					// Trigger darkening immediately, but actual effect is controlled in step
					if (!registry.deathTimers.has(player)) {
						// cease motion
						Motion& motion = registry.motions.get(player);
						motion.velocity = { 0, 0 };
						motion.is_moving_up = false;
						motion.is_moving_down = false;
						motion.is_moving_left = false;
						motion.is_moving_right = false;
						registry.deathTimers.emplace(player);
					}
				}
			}
			else {
				bounce_back(player, entity_other, scalar);
			}
		}
		if (registry.projectiles.has(entity)) {
			Projectile& projectile = registry.projectiles.get(entity);
			Entity projectileSource = projectile.source;

			// Collision logic for player projectiles hitting enemies
			if (registry.players.has(projectileSource) && registry.ais.has(entity_other)) {
				// Apply damage to the enemy
				if (registry.healths.has(entity_other)) {
					Deadly& deadly = registry.deadlies.get(entity);
					Health& enemyHealth = registry.healths.get(entity_other);
					enemyHealth.current_health -= deadly.damage;
					play_sound(enemy_hit_sound);
					if (enemyHealth.current_health <= 0) {
						// enemy is dead, trigger an explosion animation
						if (registry.motions.has(entity_other)) {
							Motion& motion = registry.motions.get(entity_other);
							createExplosion(renderer, motion.position, false); // Create explosion
						}
						registry.remove_all_components_of(entity_other); // Remove enemy if health drops to 0
						Room& current_room = registry.rooms.get(registry.players.get(player).current_room);

						// Arbitrarily remove one enemy from the internal room state when an enemy dies.
						current_room.enemy_count--;
						// remove the first element in enemy set 
						current_room.enemy_positions.erase(*current_room.enemy_positions.rbegin());
						score++;
						//registry.texts.get(score_text).content = "Score: " + std::to_string(score);
						play_sound(explosion_sound);
					}
				}
				registry.remove_all_components_of(entity); // Remove projectile after collision
			}

			// Collision logic for enemy projectiles hitting the player
			else if (registry.ais.has(projectileSource) && registry.players.has(entity_other)) {
				// Apply damage to the player
				if (registry.healths.has(entity_other)) {
					Deadly& deadly = registry.deadlies.get(entity);
					Health& playerHealth = registry.healths.get(entity_other);
					playerHealth.current_health -= 1; //hardcoded damage
					play_sound(player_hit_sound);
					if (playerHealth.current_health <= 0) {
						// Trigger darkening immediately, but actual effect is controlled in step
						if (!registry.deathTimers.has(player)) {
							// cease motion
							Motion& motion = registry.motions.get(player);
							motion.velocity = { 0, 0 };
							motion.is_moving_up = false;
							motion.is_moving_down = false;
							motion.is_moving_left = false;
							motion.is_moving_right = false;
							registry.deathTimers.emplace(player);
						}
					}
				}
				registry.remove_all_components_of(entity); // Remove projectile after collision
			}
		}
		
		// Handle collisions projectiles and obstacles
		if (registry.projectiles.has(entity) && registry.obstacles.has(entity_other)) {
            Projectile& projectile = registry.projectiles.get(entity);
            Entity projectileSource = projectile.source;

            // Check if the projectile comes from the player
            if (registry.players.has(projectileSource)) {
                // Remove the projectile, it hit an obstacle
                registry.remove_all_components_of(entity);
                continue; // Skip further processing for this collision
            }
        }
	}

	// Remove all collisions from this simulation step
	registry.collisions.clear();
}


// Should the game be over ?
bool WorldSystem::is_over() const {
	return bool(glfwWindowShouldClose(window));
}

// On key callback
void WorldSystem::on_key(int key, int, int action, int mod) 
{
	// key is of 'type' GLFW_KEY_
	// action can be GLFW_PRESS GLFW_RELEASE GLFW_REPEAT

	switch (game_state)
	{

	case GAME_STATE::START_MENU:
		// Enter key to start the game
		if (action == GLFW_RELEASE && key == GLFW_KEY_ENTER) {
			game_state = GAME_STATE::GAME;
			restart_game();
		}
		break;

	case GAME_STATE::GAME:
		// Exit the game on escape
		if (key == GLFW_KEY_ESCAPE && action == GLFW_RELEASE) {
			// TODO: Change to different screen or close depending on the game state
			glfwSetWindowShouldClose(window, GL_TRUE);
		}

		// Resetting game
		if (action == GLFW_RELEASE && key == GLFW_KEY_G) {
			int w, h;
			glfwGetWindowSize(window, &w, &h);

			restart_game();
		}

		// FPS
		if (action == GLFW_RELEASE && key == GLFW_KEY_F) {
			debugging.show_fps = !debugging.show_fps;
			if (debugging.show_fps) {
				fps_text = createText(renderer, "FPS:", { 1760.0f, 30.0f }, 0.8f, { 0.0f, 1.0f, 1.0f }, TextAlignment::LEFT);
			}
			else {
				registry.remove_all_components_of(fps_text);
			}
		}

		// Player keyboard controls
		if (!registry.deathTimers.has(player)) {
			// WEAPON CONTROLS
			if (key == GLFW_KEY_R && action == GLFW_PRESS) {
				weapons->reload_weapon();
			}
			if (key == GLFW_KEY_Q && action == GLFW_PRESS) {
				weapons->cycle_weapon(-1, registry.players.get(player)); // Cycle to the previous weapon
			}
			if (key == GLFW_KEY_E && action == GLFW_PRESS) {
				weapons->cycle_weapon(1, registry.players.get(player));  // Cycle to the next weapon
			}

			// MOVEMENT CONTROLS
			if (key == GLFW_KEY_W) {
				if (action == GLFW_PRESS) {
					registry.motions.get(player).is_moving_up = true;
				}
				else if (action == GLFW_RELEASE) {
					registry.motions.get(player).is_moving_up = false;
				}
			}
			if (key == GLFW_KEY_S) {
				if (action == GLFW_PRESS) {
					registry.motions.get(player).is_moving_down = true;
				}
				else if (action == GLFW_RELEASE) {
					registry.motions.get(player).is_moving_down = false;
				}
			}
			if (key == GLFW_KEY_A) {
				if (action == GLFW_PRESS) {
					registry.motions.get(player).is_moving_left = true;
				}
				else if (action == GLFW_RELEASE) {
					registry.motions.get(player).is_moving_left = false;
				}
			}
			if (key == GLFW_KEY_D) {
				if (action == GLFW_PRESS) {
					registry.motions.get(player).is_moving_right = true;
				}
				else if (action == GLFW_RELEASE) {
					registry.motions.get(player).is_moving_right = false;
				}
			}
		}
		break;

	case GAME_STATE::GAME_OVER:
		// Enter key to start the game
		if (action == GLFW_RELEASE && key == GLFW_KEY_ENTER) {
			game_state = GAME_STATE::GAME;
			restart_game();
		}
		break;

	default:
		break;

	}
}

void WorldSystem::bounce_back(Entity player, Entity obstacle, float scalar) {
	// Bounce back functionality by moving back by a bit
	//Motion& playerMotion = registry.motions.get(player);
	//playerMotion.velocity *= -1;
	//playerMotion.position += playerMotion.velocity * 0.01f;

	Motion& p_motion = registry.motions.get(player);
	Motion& obs_motion = registry.motions.get(obstacle);
	
	vec2 player_to_obs = obs_motion.position - p_motion.position;
	p_motion.position += glm::normalize(player_to_obs) * scalar;
	
	//vec2& p_pos = p_motion.position;
	//vec2& p_size = p_motion.scale;
	//vec2& obs_pos = obs_motion.position;
	//vec2& obs_size = obs_motion.scale;
	//
	//float p_minx = p_pos.x - p_size.x / 2;
	//float p_maxx = p_pos.x + p_size.x / 2;
	//float p_miny = p_pos.y + p_size.y / 2;
	//float p_maxy = p_pos.y - p_size.y / 2;
	//
	//float obs_left = obs_pos.x - obs_size.x / 2;
	//float obs_right = obs_pos.x + obs_size.x / 2;
	//float obs_bottom = obs_pos.y + obs_size.y / 2;
	//float obs_top = obs_pos.y - obs_size.y / 2;
	//
	//float angle = atan2(obs_pos.y - p_pos.y, obs_pos.x - p_pos.x);
	//
	//float obs_topleft_angle = atan2(obs_pos.y - obs_top, obs_pos.x - obs_left);
	//float obs_topright_angle = atan2(obs_pos.y - obs_top, obs_pos.x - obs_right);
	//float obs_bottomleft_angle = atan2(obs_pos.y - obs_bottom, obs_pos.x - obs_left);
	//float obs_bottomright_angle = atan2(obs_pos.y - obs_bottom, obs_pos.x - obs_right);
	//
	//// Check if player travels upwards
	//if (angle > obs_bottomright_angle && angle <= obs_bottomleft_angle) {
	//	p_pos.y = obs_bottom + p_size.y / 2;
	//	p_motion.velocity.y = 0;
	//}
	//// Check if player travels downwards
	//else if (angle > obs_topleft_angle && angle <= obs_topright_angle) {
	//	p_pos.y = obs_top - p_size.y / 2;
	//	p_motion.velocity.y = 0;
	//}
	//// Check if player travels rightwards
	//else if (angle > obs_bottomleft_angle && angle <= obs_topleft_angle) {	
	//	p_pos.x = obs_left - p_size.x / 2;
	//	p_motion.velocity.x = 0;
	//}
	//// Check if player travels leftwards
	//else if (angle >= obs_topright_angle || angle <= obs_bottomright_angle) {
	//	p_pos.x = obs_right + p_size.x / 2;
	//	p_motion.velocity.x = 0;
	//}
}

void WorldSystem::on_mouse_move(vec2 mouse_position) 
{
	switch (game_state) 
	{

	case GAME_STATE::START_MENU:
		break;
	
	case GAME_STATE::GAME:
		if (registry.deathTimers.has(player)) {
			return;
		}
		vec2 player_position = registry.motions.get(player).position;
		vec2 direction = mouse_position - player_position;
		registry.motions.get(player).look_angle = atan2(direction.y, direction.x) + M_PI/2;
		break;
	
	case GAME_STATE::GAME_OVER:
		break;
	
	default:
		break;

	}
}

void WorldSystem::on_mouse_click(int button, int action, int mods) 
{
	// glfw mouse button codes
	int left_click = 0;
	int right_click = 1;
	int press = 1;
	int release = 0;

	switch (game_state)
	{

	case GAME_STATE::START_MENU:
		break;

	case GAME_STATE::GAME:
		if (registry.deathTimers.has(player)) {
			return;
		}

		if (button == left_click) {
			if (action == press) {
				registry.players.get(player).is_firing = true;
			}
			else if (action == release) {
				registry.players.get(player).is_firing = false;
				registry.players.get(player).fire_length_ms = 0.0f;
			}
		}
		break;

	case GAME_STATE::GAME_OVER:
		break;

	default:
		break;

	}
}

void WorldSystem::fpsCalculate() {
	//average these many samples to change the frames smoother
	static const int num_samples = 10; 
	//buffer, array of the frames
	static float frameTimes[num_samples];
	static int currentFrame = 0;

	static float prevTicks = SDL_GetTicks();
	
	float currentTicks;
	currentTicks = SDL_GetTicks();

	frameTime = currentTicks - prevTicks;//note: first few ticks will be wrong
	frameTimes[currentFrame % num_samples] = frameTime; 

	//need to set ticks again. after current tick is done, it becomes prev ticks
	prevTicks = currentTicks;
	int count;
//	currentFrame++;

	if (currentFrame < num_samples) {
		count = currentFrame;
	}
	else {
		count = num_samples;
	}

	float averageFrameTime = 0;
	for (int i = 0; i < count; i++) {
		averageFrameTime += frameTimes[i];

	}
	averageFrameTime /= count;

	if (averageFrameTime > 0) {
		fps = 1000.0f / averageFrameTime;
		// ms/s divided by ms/f returns frames/second which is fps
	}
	else {
		//shouldn't ever reach this else case 
		fps = 60.0f;
	}
	currentFrame++;
}
