// Header
#include "world_system/world_system.hpp"
#include "world_init/world_init.hpp"
#include "physics_system/physics_system.hpp"
#include "ui_system/ui_system.hpp"
#include "weapon_system/weapon_system.hpp"

// stlib
#include <cassert>
#include <sstream>
#include <iostream>
#include <glm/trigonometric.hpp>

// Game configuration
// TODO: set hard coded game configuration values here
bool fullscreen;

// Create the world
WorldSystem::WorldSystem()
{
	// TODO: world initialization here
	fullscreen = 0;
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


	//	window = glfwCreateWindow(window_width_px, window_height_px, "Touch of the Void", glfwGetPrimaryMonitor(), nullptr);

	// Create the main window (for rendering, keyboard, and mouse input)

	window = glfwCreateWindow(window_width_px, window_height_px, "Touch of the Void", glfwGetPrimaryMonitor(), nullptr);

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

	// Set all states to default
	restart_game();
}

bool WorldSystem::progress_timers(Player& player, float elapsed_ms_since_last_update) {
	ScreenState& screen = registry.screenStates.components[0];
	//std::cout << "Screen state brightness: " << screen.darken_screen_factor << std::endl;
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
	/*	if (counter.counter_ms < min_counter_ms) {
			min_counter_ms = counter.counter_ms;
		}*/

		// remove the darken effect once the timer expired
		if (counter.counter_ms < 0) {
			registry.roomTransitionTimers.remove(entity);
			screen.darken_screen_factor = 0;
			player.is_moving_rooms = false;
			// player enters new room
			//return true;
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

	for (Entity entity : registry.damagedTimers.entities) {
		// progress timer
		DamagedTimer& counter = registry.damagedTimers.get(entity);
		counter.counter_ms -= elapsed_ms_since_last_update;
		if (counter.counter_ms < min_counter_ms) {
			min_counter_ms = counter.counter_ms;
		}

		// remove the damaged effect once the timer expired
		if (counter.counter_ms < 0) {
			registry.damagedTimers.remove(entity);
		}
	}
	return false;
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
	ui->update(registry.healths.get(player), registry.shields.get(player), registry.players.get(player), score, multiplier, 0, debugging.show_fps, registry.levels.get(level));
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

	// Progress game timers
	if (progress_timers(p, elapsed_ms_since_last_update)) { // if the timers returned true, then we should return true early
		return true;
	}

	// heal player shield over timne
	if (registry.shields.has(player) && !registry.damagedTimers.has(player)) {
		Shield& playerShield = registry.shields.get(player);
		if (playerShield.current_shield < playerShield.max_shield) {
			playerShield.current_shield += playerShield.recharge_rate * elapsed_ms_since_last_update * 0.01f;
			playerShield.current_shield = std::min(playerShield.current_shield, playerShield.max_shield); // don't exceed max shield
		}

	}

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

	// Stop playing music
	stop_music();

	switch (game_state) 
	{
	case GAME_STATE::START_MENU:
		play_music(start_menu_music);

		/*createText(renderer, "TOUCH OF THE VOID", { 960.0f, 324.0f }, 3.f, COLOR_RED, TextAlignment::CENTER);
		createText(renderer, "Press 'enter' to start", { 960.0f, 464.0f }, 1.f, COLOR_WHITE, TextAlignment::CENTER);*/

		createStartScreen(renderer);
		//// Tutorial Text
		createText(renderer, "CONTROLS", { 30.0f, 674.0f }, 1.4f, COLOR_WHITE, TextAlignment::LEFT);
		createText(renderer, "WASD to move", { 30.0f, 714.0f }, 0.7f, COLOR_WHITE, TextAlignment::LEFT);
		createText(renderer, "Mouse to aim", { 30.0f, 754.0f }, 0.7f, COLOR_WHITE, TextAlignment::LEFT);
		createText(renderer, "Right-Click to shoot", { 30.0f, 794.0f }, 0.7f, COLOR_WHITE, TextAlignment::LEFT);
		createText(renderer, "R to reload", { 30.0f, 834.0f }, 0.7f, COLOR_WHITE, TextAlignment::LEFT);
		createText(renderer, "Q/E to change weapons", { 30.0f, 874.0f }, 0.7f, COLOR_WHITE, TextAlignment::LEFT);

		break;

	case GAME_STATE::GAME:
		play_music(game_music);

		// Create a new player
		player = createPlayer(renderer, { window_width_px / 2, window_height_px / 2 });

		// Create a level
		createBackground(renderer);
		level = createLevel(renderer);

		//// Create HUD
		score = 100;
		multiplier = 1.0;
		ui->init(renderer, registry.healths.get(player), registry.shields.get(player), registry.players.get(player), score, multiplier, registry.levels.get(level));
		break;

	case GAME_STATE::GAME_OVER:
		/*createText(renderer, "GAME OVER", { 960.0f, 324.0f }, 3.f, COLOR_RED, TextAlignment::CENTER);
		createText(renderer, "Press 'enter' to play again", { 960.0f, 464.0f }, 1.f, COLOR_WHITE, TextAlignment::CENTER);*/
		createDeathScreen(renderer);
		break;

	default:
		break;
	}
}



void WorldSystem::enter_room(vec2 player_pos) {
	// Debugging for memory/component leaks
	registry.list_all_components();
	printf("Entering Room\n");


	// Reset darken_screen_factor on room enter
	ScreenState& screen = registry.screenStates.components[0];
	screen.darken_screen_factor = 0.0f;

	remove_entities_on_entry();
	

	// Debugging for memory/component leaks
	registry.list_all_components();

	// Render the room
	Level& level_struct = registry.levels.get(level);
	render_room(renderer, level_struct);

	// Move the player to position
	registry.motions.get(player).position = player_pos;
}
// Remove entities between rooms
 void WorldSystem::remove_entities_on_entry()
{
	for (Entity e : registry.motions.entities)
	{
		// remove all enemies, obstacles, animations
		if (registry.obstacles.has(e) || registry.deadlies.has(e) || registry.animations.has(e))
		{
			registry.remove_all_components_of(e);
		}
	}
}
// Compute collisions between entities
void WorldSystem::handle_collisions() {
	// Loop over all collisions detected by the physics system
	auto& collisionsRegistry = registry.collisions;
	for (uint i = 0; i < collisionsRegistry.components.size(); i++) {
		// The entity and its collider
		Entity entity = collisionsRegistry.entities[i];
		Entity entity_other = collisionsRegistry.components[i].other;

		if (registry.players.has(entity) && registry.obstacles.has(entity_other)) {
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
					float x_delta = game_window_size_px / 2 - 32;
					float y_delta = game_window_size_px / 2 - 32;
					float x_max = x_mid + x_delta;
					float x_min = x_mid - x_delta;
					float y_max = y_mid + y_delta;
					float y_min = y_mid - y_delta;
					// change to the room that they entered
					if (obstacle.is_bottom_door)
					{
						Level& level_struct = registry.levels.get(level);
						level_struct.current_room = std::pair<int, int>(level_struct.current_room.first, level_struct.current_room.second - 1);
						next_pos = { x_mid, y_min + 64 };
					}
					else if (obstacle.is_top_door)
					{
						Level& level_struct = registry.levels.get(level);
						level_struct.current_room = std::pair<int, int>(level_struct.current_room.first, level_struct.current_room.second + 1);
						next_pos = { x_mid, y_max - 64 };
					}
					else if (obstacle.is_left_door)
					{
						Level& level_struct = registry.levels.get(level);
						level_struct.current_room = std::pair<int, int>(level_struct.current_room.first - 1, level_struct.current_room.second);
						next_pos = { x_max - 64, y_mid };
					}
					else if (obstacle.is_right_door)
					{
						Level& level_struct = registry.levels.get(level);
						level_struct.current_room = std::pair<int, int>(level_struct.current_room.first + 1, level_struct.current_room.second);
						next_pos = { x_min + 64, y_mid };
					}
					Level& level_struct = registry.levels.get(level);
			
					registry.roomTransitionTimers.emplace(entity);
					ScreenState& screen = registry.screenStates.components[0];
					screen.darken_screen_factor = 1.0f;
					enter_room(next_pos);
				}
			}

			// Apply damage to the player
			else if (registry.deadlies.has(entity_other)) {
				assert(registry.shields.has(entity) && "Player should have a shield");
				Shield& playerShield = registry.shields.get(player);

				if (registry.damagedTimers.has(player)) {
					DamagedTimer& damagedTimer = registry.damagedTimers.get(player);
					damagedTimer.counter_ms = playerShield.recharge_delay;
				}
				else {
					DamagedTimer& damagedTimer = registry.damagedTimers.emplace(player);
					damagedTimer.counter_ms = playerShield.recharge_delay;
				}

				if (playerShield.current_shield > 0) {
					play_sound(player_hit_sound);
					playerShield.current_shield -= registry.deadlies.get(entity_other).damage;
					playerShield.current_shield = std::max(playerShield.current_shield, 0.0f);
				}
				else {
					assert(registry.healths.has(player) && "Player should have health");
					assert(registry.deadlies.has(entity_other) && "Entity should have a deadly component");
					Deadly& deadly = registry.deadlies.get(entity_other);
					Health& playerHealth = registry.healths.get(player);
					playerHealth.current_health -= 1; //hardcoded damage
					if (playerHealth.current_health <= 0) {
						// Trigger darkening immediately, but actual effect is controlled in step
						if (!registry.deathTimers.has(player)) {
							// cease motion
							assert(registry.motions.has(player) && "Player should have a motion");
							Motion& motion = registry.motions.get(player);
							motion.velocity = { 0, 0 };
							motion.is_moving_up = false;
							motion.is_moving_down = false;
							motion.is_moving_left = false;
							motion.is_moving_right = false;
							registry.deathTimers.emplace(player);
							play_sound(game_over_sound);
						}
					}
					else {
						play_sound(player_hit_sound);
					}
				}
			}
			else {
				bounce_back(player, entity_other);
			}
		}

		// PROJECTILE COLLISONS
		if (registry.projectiles.has(entity)) 
		{
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

					switch (projectile.weapon_type) 
					{
					case WeaponType::ROCKET_LAUNCHER:
						weapons->handle_rocket_collision(renderer, entity, player);
						break;

					case WeaponType::FLAMETHROWER:
						weapons->handle_flamethrower_collision(renderer, entity, entity_other);
						break;
					
					default:
						createBulletImpact(renderer, registry.motions.get(entity).position, 1.0, false);
					}
				}
				registry.remove_all_components_of(entity); // Remove projectile after collision
			}

			// Collision logic for enemy projectiles hitting the player
			else if (registry.ais.has(projectileSource) && registry.players.has(entity_other)) {
				// Apply damage to the player
				if (registry.healths.has(entity_other) && registry.shields.has(entity_other)) {
					assert(registry.shields.has(entity_other) && "Player should have a shield");
					Shield& playerShield = registry.shields.get(entity_other);

					if (registry.damagedTimers.has(entity_other)) {
						DamagedTimer& damagedTimer = registry.damagedTimers.get(entity_other);
						damagedTimer.counter_ms = playerShield.recharge_delay;
					}
					else {
						DamagedTimer& damagedTimer = registry.damagedTimers.emplace(player);
						damagedTimer.counter_ms = playerShield.recharge_delay;
					}

					if (playerShield.current_shield > 0) {
						play_sound(player_hit_sound);
						playerShield.current_shield -= 10.f; // registry.deadlies.get(entity).damage;
						playerShield.current_shield = std::max(playerShield.current_shield, 0.0f);
					}
					else {
						assert(registry.healths.has(entity_other) && "Player should have health");
						assert(registry.deadlies.has(entity) && "Entity should have a deadly component");
						Deadly& deadly = registry.deadlies.get(entity);
						Health& playerHealth = registry.healths.get(entity_other);
						playerHealth.current_health -= 1; //hardcoded damage
						if (playerHealth.current_health <= 0) {
							// Trigger darkening immediately, but actual effect is controlled in step
							if (!registry.deathTimers.has(player)) {
								// cease motion
								assert(registry.motions.has(player) && "Player should have a motion");
								Motion& motion = registry.motions.get(player);
								motion.velocity = { 0, 0 };
								motion.is_moving_up = false;
								motion.is_moving_down = false;
								motion.is_moving_left = false;
								motion.is_moving_right = false;
								registry.deathTimers.emplace(player);
								play_sound(game_over_sound);
							}
						}
						else {
							play_sound(player_hit_sound);
						}
					}
					
				}
				registry.remove_all_components_of(entity); // Remove projectile after collision
			}

			// Collision logic for player projectiles hitting obstacles
			else if (registry.projectiles.has(entity) && registry.obstacles.has(entity_other)) {
				Projectile& projectile = registry.projectiles.get(entity);
				Entity projectileSource = projectile.source;

				// Check if the projectile comes from the player
				if (registry.players.has(projectileSource)) {
					if (projectile.weapon_type == WeaponType::ROCKET_LAUNCHER) {
						weapons->handle_rocket_collision(renderer, entity, player);
					}
					// Remove the projectile, it hit an obstacle
					registry.remove_all_components_of(entity);
				}
			}
		}
	}

	// Check for dead enemies
	for (Entity e : registry.ais.entities) {
		vec2 e_pos = registry.motions.get(e).position;
		Health& e_health = registry.healths.get(e);

		if (e_health.current_health <= 0) {

			// remove the fire effect if an enemy dies
			if (registry.onFireTimers.has(e)) {
				registry.remove_all_components_of(registry.onFireTimers.get(e).fire);
			}

			registry.remove_all_components_of(e);
			Level& current_level = registry.levels.get(level);
			Room& current_room = registry.rooms.get(current_level.rooms[current_level.current_room]);
			// Arbitrarily remove one enemy from the internal room state when an enemy dies.
			current_room.enemy_count--;
			// remove the first element in enemy set 
			current_room.enemy_positions.erase(*current_room.enemy_positions.rbegin());
			score++;

			if (current_room.enemy_count == 0)
			{
				registry.levels.get(level).num_rooms_until_boss--;
				current_room.has_bottom_door = true;
				current_room.has_top_door = true;
				current_room.has_left_door = true;
				current_room.has_right_door = true;

				// tear down existing walls
				clearExistingWalls();
				// re-render walls with doors
				createWalls(renderer, current_room);
			}
			// UX Effects
			createExplosion(renderer, e_pos, 1.0f, false);
			play_sound(explosion_sound);
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
			play_sound(game_start_sound);
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
		}
		//full screen mode

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
			play_sound(game_start_sound);
			restart_game();
		}
		break;

	default:
		break;

	}
}

void WorldSystem::bounce_back(Entity player, Entity obstacle) {
	// Bounce back functionality by moving back by a bit
	//Motion& playerMotion = registry.motions.get(player);
	//playerMotion.velocity *= -1;
	//playerMotion.position += playerMotion.velocity * 0.01f;

	Motion& p_motion = registry.motions.get(player);
	Motion& obs_motion = registry.motions.get(obstacle);

	vec2& p_pos = p_motion.position;
	vec2& p_size = p_motion.scale;
	vec2& obs_pos = obs_motion.position;
	vec2& obs_size = obs_motion.scale;

	float p_minx = p_pos.x - p_size.x / 2;
	float p_maxx = p_pos.x + p_size.x / 2;
	float p_miny = p_pos.y + p_size.y / 2;
	float p_maxy = p_pos.y - p_size.y / 2;

	float obs_left = obs_pos.x - obs_size.x / 2;
	float obs_right = obs_pos.x + obs_size.x / 2;
	float obs_bottom = obs_pos.y + obs_size.y / 2;
	float obs_top = obs_pos.y - obs_size.y / 2;
 
	float angle = atan2(obs_pos.y - p_pos.y, obs_pos.x - p_pos.x);
	
	float obs_topleft_angle = atan2(obs_pos.y - obs_top, obs_pos.x - obs_left);
	float obs_topright_angle = atan2(obs_pos.y - obs_top, obs_pos.x - obs_right);
	float obs_bottomleft_angle = atan2(obs_pos.y - obs_bottom, obs_pos.x - obs_left);
	float obs_bottomright_angle = atan2(obs_pos.y - obs_bottom, obs_pos.x - obs_right);

	// Check if player travels upwards
	if (angle > obs_bottomright_angle && angle <= obs_bottomleft_angle) {
		p_pos.y = obs_bottom + p_size.y / 2;
		p_motion.velocity.y = 0;
	}
	// Check if player travels downwards
	else if (angle > obs_topleft_angle && angle <= obs_topright_angle) {
		p_pos.y = obs_top - p_size.y / 2;
		p_motion.velocity.y = 0;
	}
	// Check if player travels rightwards
	else if (angle > obs_bottomleft_angle && angle <= obs_topleft_angle) {	
		p_pos.x = obs_left - p_size.x / 2;
		p_motion.velocity.x = 0;
	}
	// Check if player travels leftwards
	else if (angle >= obs_topright_angle || angle <= obs_bottomright_angle) {
		p_pos.x = obs_right + p_size.x / 2;
		p_motion.velocity.x = 0;
	}
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
