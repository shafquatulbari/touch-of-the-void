// Header
#include "world_system/world_system.hpp"
#include "world_init/world_init.hpp"

// stlib
#include <cassert>
#include <sstream>
#include <iostream>

#include "physics_system/physics_system.hpp"

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
	// TODO: destroy any music components

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
	if (SDL_Init(SDL_INIT_AUDIO) < 0) {
		fprintf(stderr, "Failed to initialize SDL Audio");
		return nullptr;
	}
	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) == -1) {
		fprintf(stderr, "Failed to open audio device");
		return nullptr;
	}

	// Load music and sounds
	// TODO: Load our music and sounds as so.
	
	// background_music = Mix_LoadMUS(audio_path("music.wav").c_str());
	// chicken_dead_sound = Mix_LoadWAV(audio_path("chicken_dead.wav").c_str());

	//if (background_music == nullptr || chicken_dead_sound == nullptr) {
	//	fprintf(stderr, "Failed to load sounds\n %s\n %s\n make sure the data directory is present",
	//		audio_path("music.wav").c_str(),
	//		audio_path("chicken_dead.wav").c_str());
	//	return nullptr;
	//}


	return window;
}

void WorldSystem::init(RenderSystem* renderer_arg) {
	this->renderer = renderer_arg;
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
bool WorldSystem::step(float elapsed_ms_since_last_update) {

	// Remove debug info from the last step
	while (registry.debugComponents.entities.size() > 0)
		registry.remove_all_components_of(registry.debugComponents.entities.back());

	// Update HUD
	int roundedHealth = static_cast<int>(registry.healths.get(player).current_health); // round to nearest int so the HUD doesn't get cluttered
	std::string healthText = "HP: " + std::to_string(roundedHealth) + " / 100";
	registry.texts.get(player_hp_text).content = healthText;

	auto& motions_registry = registry.motions;

	Player& p = registry.players.get(player);
	// Handle player firing
	if (p.is_reloading) {
		registry.texts.get(ammo_text).content = "Ammo: Reloading...";
		p.reload_timer_ms -= elapsed_ms_since_last_update;

		if (p.reload_timer_ms < 0) {
			// Reload complete, refill ammo
			p.is_reloading = false;
			p.reload_timer_ms = p.reload_times[p.weapon_type];
			p.ammo_count = p.magazine_sizes[p.weapon_type]; // Refill ammo after reload
			registry.texts.get(ammo_text).content = "Ammo: " + std::to_string(p.ammo_count) + " / " + std::to_string(p.magazine_sizes[p.weapon_type]);
		}
	}

	// Removing out of screen entities
	// Remove entities that leave the screen on the left side
	// Iterate backwards to be able to remove without unterfering with the next object to visit
	// (the containers exchange the last element with the current)
	for (int i = (int)motions_registry.components.size() - 1; i >= 0; --i) {
		Motion& motion = motions_registry.components[i];
		if (motion.position.x + abs(motion.scale.x) < 0.f) {
			if (!registry.players.has(motions_registry.entities[i])) // don't remove the player
				registry.remove_all_components_of(motions_registry.entities[i]);
		}

		if (registry.projectiles.has(motions_registry.entities[i])) {
			// max_position from physics_system.cpp and replaced game_window_block_size 
			// with the entity's width and height
			float max_position_x = (game_window_size_px / 2) - (motion.scale.x / 2);
			float max_position_y = (game_window_size_px / 2) - (motion.scale.y / 2);
			if (
				abs(motion.position.x - (window_width_px / 2)) >= max_position_x ||
				abs(motion.position.y - (window_height_px / 2)) >= max_position_y
			)
					registry.remove_all_components_of(motions_registry.entities[i]);
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
			enter_room(registry.rooms.get(p.current_room)
				, { window_width_px / 2, window_height_px / 2});
			return true;
		}
	}
	// reduce window brightness if any of the player is dying
	screen.darken_screen_factor = 1 - min_counter_ms / 3000;

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
		registry.texts.get(fps_text).content = "FPS: " + std::to_string(static_cast<int>(fps));
		frameCounter = 0;
	}
	float frameTicks = SDL_GetTicks() - startTicks;

	//fps limiter. 
	/*if ((1000.0f / maxFps) > frameTicks) {
		SDL_Delay((1000.0f / maxFps) - frameTicks);
	}*/

	return true;
}

// Reset the world state to its initial state
void WorldSystem::restart_game() {
	// Debugging for memory/component leaks
	registry.list_all_components();
	printf("Restarting\n");

	// Reset the game speed
	current_speed = 1.f;

	// Reset darken_screen_factor on game restart
    ScreenState& screen = registry.screenStates.components[0];
    screen.darken_screen_factor = 0.0f; 

	// Remove all entities that we created
	// All that have a motion, we could also iterate over all bug, eagles, ... but that would be more cumbersome
	while (registry.motions.entities.size() > 0)
		registry.remove_all_components_of(registry.motions.entities.back());

	// Debugging for memory/component leaks
	registry.list_all_components();

	// Create a level
	Entity starting_room = createBackground(renderer);

	// Create a new player
	player = createPlayer(renderer, { window_width_px / 2, window_height_px / 2 });
	registry.players.get(player).current_room = starting_room;
	// Tutorial Text
	createText(renderer, "CONTROLS", { 20.0f, 440.0f }, 0.7f, { 1.0f, 1.0f, 1.0f });
	createText(renderer, "WASD to move", { 20.0f, 400.0f }, 0.4f, { 1.0f, 1.0f, 1.0f });
	createText(renderer, "Mouse to aim", { 20.0f, 370.0f }, 0.4f, { 1.0f, 1.0f, 1.0f });
	createText(renderer, "Right-Click to shoot", { 20.0f, 340.0f }, 0.4f, { 1.0f, 1.0f, 1.0f });
	createText(renderer, "R to reload", { 20.0f, 310.0f }, 0.4f, { 1.0f, 1.0f, 1.0f });
	createText(renderer, "Q/E to change weapons", { 20.0f, 280.0f }, 0.4f, { 1.0f, 1.0f, 1.0f });

	// Create HUD
	player_hp_text = createText(renderer, "HP: 100 / 100", { 780.0f, 400.0f }, 0.5f, { 1.0f, 0.15f, 0.15f });
	weapon_text = createText(renderer, "Weapon: Machine Gun", {780.0f, 360.0f}, 0.5f, { 0.26f, 0.97f, 0.19f });
	ammo_text = createText(renderer, "Ammo: 30 / 30", { 780.0f, 320.0f }, 0.5f, { 0.26f, 0.97f, 0.19f });

	// FPS
	fps_text = createText(renderer, "FPS:", { 920.0f, 480.0f }, 0.5f, { 0.0f, 1.0f, 1.0f });
}


// Reset the world state to its initial state
void WorldSystem::enter_room(Room& room, vec2 player_pos) {
	// Debugging for memory/component leaks
	registry.list_all_components();
	printf("Entering Room\n");

	// Reset the game speed
	current_speed = 1.f;

	// Reset darken_screen_factor on room enter
	ScreenState& screen = registry.screenStates.components[0];
	screen.darken_screen_factor = 0.0f;

	for(Entity e: registry.motions.entities)
	{
		// clear motion registry except for player.
		if (!registry.players.has(e))
		{
			registry.remove_all_components_of(e);
		}
	}

	// Debugging for memory/component leaks
	registry.list_all_components();

	// Render the room
	render_room(renderer, room);

	// Move the player to position
	registry.motions.get(player).position = player_pos;


	// Tutorial Text
	createText(renderer, "CONTROLS", { 20.0f, 440.0f }, 0.7f, { 1.0f, 1.0f, 1.0f });
	createText(renderer, "WASD to move", { 20.0f, 400.0f }, 0.4f, { 1.0f, 1.0f, 1.0f });
	createText(renderer, "Mouse to aim", { 20.0f, 370.0f }, 0.4f, { 1.0f, 1.0f, 1.0f });
	createText(renderer, "Right-Click to shoot", { 20.0f, 340.0f }, 0.4f, { 1.0f, 1.0f, 1.0f });
	createText(renderer, "R to reload", { 20.0f, 310.0f }, 0.4f, { 1.0f, 1.0f, 1.0f });
	createText(renderer, "Q/E to change weapons", { 20.0f, 280.0f }, 0.4f, { 1.0f, 1.0f, 1.0f });

	// Create HUD
	player_hp_text = createText(renderer, "HP: 100 / 100", { 780.0f, 400.0f }, 0.5f, { 1.0f, 0.15f, 0.15f });
	weapon_text = createText(renderer, "Weapon: Machine Gun", { 780.0f, 360.0f }, 0.5f, { 0.26f, 0.97f, 0.19f });
	ammo_text = createText(renderer, "Ammo: 30 / 30", { 780.0f, 320.0f }, 0.5f, { 0.26f, 0.97f, 0.19f });

	// FPS
	fps_text = createText(renderer, "FPS:", { 920.0f, 480.0f }, 0.5f, { 0.0f, 1.0f, 1.0f });
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
					// change to the room that they entered
					if (obstacle.is_bottom_door)
					{
						player.current_room = registry.rooms.get(player.current_room).bottom_room;
					}
					else if (obstacle.is_top_door)
					{
						player.current_room = registry.rooms.get(player.current_room).top_room;
					}
					else if (obstacle.is_left_door)
					{
						player.current_room = registry.rooms.get(player.current_room).left_room;
					}
					else if (obstacle.is_right_door)
					{
						player.current_room = registry.rooms.get(player.current_room).right_room;
					}
				
					// darken effect
					registry.roomTransitionTimers.emplace(entity);
				}
				
			}
			// Apply damage to the player
			Health& playerHealth = registry.healths.get(player);
			if (registry.deadlies.has(entity_other)) {
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
			bounce_back(player, entity_other);
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
					if (enemyHealth.current_health <= 0) {
						registry.remove_all_components_of(entity_other); // Remove enemy if health drops to 0
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

			// Additional collision handling logic can be added here
		}
		/*
		// Handle collisions projectiles and obstacles
		if (registry.projectiles.has(entity) && registry.obstacles.has(entity_other)) {
				Projectile& projectile = registry.projectiles.get(entity);
				if (registry.healths.has(entity_other)) {
					Deadly& deadly = registry.deadlies.get(entity);
					Health& obstacle_health = registry.healths.get(entity_other);
					obstacle_health.current_health -= deadly.damage;
					if (obstacle_health.current_health <= 0) {
						registry.remove_all_components_of(entity_other);
					}
				}
				registry.remove_all_components_of(entity);
		}
		*/
	}

	// Remove all collisions from this simulation step
	registry.collisions.clear();
}


// Should the game be over ?
bool WorldSystem::is_over() const {
	return bool(glfwWindowShouldClose(window));
}

// On key callback
void WorldSystem::on_key(int key, int, int action, int mod) {
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// TODO: HANDLE PLAYER MOVEMENT HERE
	// key is of 'type' GLFW_KEY_
	// action can be GLFW_PRESS GLFW_RELEASE GLFW_REPEAT
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

	// Resetting game
	if (action == GLFW_RELEASE && key == GLFW_KEY_G) {
		int w, h;
		glfwGetWindowSize(window, &w, &h);

		restart_game();
	}

	// Debugging
	if (key == GLFW_KEY_D) {
		if (action == GLFW_RELEASE)
			debugging.in_debug_mode = false;
		else
			debugging.in_debug_mode = true;
	}

	// Control the current speed with `<` `>`
	if (action == GLFW_RELEASE && (mod & GLFW_MOD_SHIFT) && key == GLFW_KEY_COMMA) {
		current_speed -= 0.1f;
		printf("Current speed = %f\n", current_speed);
	}
	if (action == GLFW_RELEASE && (mod & GLFW_MOD_SHIFT) && key == GLFW_KEY_PERIOD) {
		current_speed += 0.1f;
		printf("Current speed = %f\n", current_speed);
	}
	current_speed = fmax(0.f, current_speed);

	// exit the game on escape
	if (key == GLFW_KEY_ESCAPE && action == GLFW_RELEASE) {
		// TODO: Change to different screen or close depending on the game state
		glfwSetWindowShouldClose(window, GL_TRUE);
	}


	if (!registry.deathTimers.has(player)) {
		// TODO: Handle player controls here

		// WEAPON CONTROLS
		if (key == GLFW_KEY_R && action == GLFW_PRESS) {
			registry.players.get(player).is_reloading = true;
		}
		if (key == GLFW_KEY_Q && action == GLFW_PRESS) {
			cycle_weapon(-1);  // Cycle to the previous weapon
		}
		if (key == GLFW_KEY_E && action == GLFW_PRESS) {
			cycle_weapon(1);  // Cycle to the next weapon
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

}

// Function to cycle player weapons (-1 for previous, 1 for next)
void WorldSystem::cycle_weapon(int direction) {
	Player& p = registry.players.get(player);

	// Get the current player's weapon type
	Player::WeaponType currentWeapon = registry.players.get(player).weapon_type;

	// Get the total number of weapon types
	int numWeapons = static_cast<int>(Player::WeaponType::TOTAL_WEAPON_TYPES);

	// Calculate the new weapon index
	int newWeaponIndex = static_cast<int>(currentWeapon) + direction;
	if (newWeaponIndex < 0) {
		newWeaponIndex = numWeapons - 1;  // Wrap around to the last weapon
	}
	else if (newWeaponIndex >= numWeapons) {
		newWeaponIndex = 0;  // Wrap around to the first weapon
	}

	// Set the new weapon type
	Player::WeaponType newWeapon = static_cast<Player::WeaponType>(newWeaponIndex);
	registry.players.get(player).weapon_type = newWeapon;
	
	// Convert the enum value to a string for printing
	std::string weaponString;
	switch (newWeapon) {
	case Player::WeaponType::MACHINE_GUN:
		weaponString = "Weapon: Machine Gun";
		break;
	case Player::WeaponType::SNIPER:
		weaponString = "Weapon: Sniper";
		break;
	case Player::WeaponType::SHOTGUN:
		weaponString = "Weapon: Shotgun";
		break;
	default:
		weaponString = "Unknown Weapon";
		break;
	}

	// Update ammo counters and reload timers
	p.is_reloading = true;
	p.reload_timer_ms = p.reload_times[p.weapon_type];
	p.ammo_count = p.magazine_sizes[p.weapon_type];
	registry.texts.get(weapon_text).content = weaponString;
	registry.texts.get(ammo_text).content = "Ammo: " + std::to_string(p.ammo_count) + " / " + std::to_string(p.magazine_sizes[p.weapon_type]);
}

void WorldSystem::bounce_back(Entity player, Entity obstacle) {
	// Bounce back functionality by moving back by a bit
	Motion& playerMotion = registry.motions.get(player);
	playerMotion.velocity *= -1;
	playerMotion.position += playerMotion.velocity * 0.01f;
}

void WorldSystem::on_mouse_move(vec2 mouse_position) {
	if (registry.deathTimers.has(player)) {
		return;
	}
	vec2 player_position = registry.motions.get(player).position;
	vec2 direction = mouse_position - player_position;
	registry.motions.get(player).look_angle = atan2(direction.y, direction.x) + M_PI/2;
}

void WorldSystem::on_mouse_click(int button, int action, int mods) {
	if (registry.deathTimers.has(player)) {
		return;
	}

	// glfw mouse button codes
	int left_click = 0;
	int right_click = 1;
	int press = 1;
	int release = 0;

	if (button == left_click) {
		if (action == press) {
			if (!registry.players.get(player).is_reloading) {
				switch (registry.players.get(player).weapon_type) {
				case Player::WeaponType::MACHINE_GUN:
					// TODO: Logic for firing machine gun
					createProjectile(
						renderer,
						registry.motions.get(player).position,
						registry.motions.get(player).look_angle - M_PI / 2,
						uniform_dist(rng),
						registry.players.get(player).fire_length_ms,
						player);
					break;

				case Player::WeaponType::SNIPER:
					// TODO: Logic for firing sniper
					createProjectile(
						renderer,
						registry.motions.get(player).position,
						registry.motions.get(player).look_angle - M_PI / 2,
						uniform_dist(rng),
						registry.players.get(player).fire_length_ms,
						player);
					break;

				case Player::WeaponType::SHOTGUN:
					// TODO: Logic for firing shotgun
					createProjectile(
						renderer,
						registry.motions.get(player).position,
						registry.motions.get(player).look_angle - M_PI / 2,
						uniform_dist(rng),
						registry.players.get(player).fire_length_ms,
						player);
					break;

				default:
					// Handle an unknown weapon type (should never reach here, hopefully...)
					break;
				}
				std::cout << registry.players.get(player).ammo_count << std::endl;
				registry.players.get(player).ammo_count -= 1;

				registry.texts.get(ammo_text).content =
					"Ammo: " +
					std::to_string(registry.players.get(player).ammo_count) +
					" / " +
					std::to_string(registry.players.get(player).magazine_sizes[registry.players.get(player).weapon_type]);

				if (registry.players.get(player).ammo_count <= 0) {
					registry.players.get(player).is_reloading = true;
				}

				// player is firing 
				//registry.players.get(player).is_firing = true;
			}
		}
		else if (action == release) {
			registry.players.get(player).is_firing = false;
			registry.players.get(player).fire_length_ms = 0.0f;
		}
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
