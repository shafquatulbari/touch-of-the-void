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
	std::stringstream title_ss;
	title_ss << "Touch of the Void";
	glfwSetWindowTitle(window, title_ss.str().c_str());
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

	// Removing out of screen entities
	auto& motions_registry = registry.motions;

	if (registry.players.get(player).is_firing) {
		// increase the counter of fire length
		registry.players.get(player).fire_length_ms += elapsed_ms_since_last_update;
		createProjectile(renderer, motions_registry.get(player).position, motions_registry.get(player).look_angle - M_PI / 2, uniform_dist(rng), registry.players.get(player).fire_length_ms);
	}
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
	createBackground(renderer);

	// Create a new player
	player = createPlayer(renderer, { window_width_px / 2, window_height_px / 2 });
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
				//on collision with an obstacle, the player should take damage and bounce back
				apply_damage_and_bounce_back(entity, entity_other);
			}

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
	if (action == GLFW_RELEASE && key == GLFW_KEY_R) {
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
			// TODO: initiate reload
		}
		if (key == GLFW_KEY_Q && action == GLFW_PRESS) {
			// TODO: Change weapon (scroll up)
		}
		if (key == GLFW_KEY_E && action == GLFW_PRESS) {
			// TODO: Change weapon (scroll down)
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

void WorldSystem::apply_damage_and_bounce_back(Entity player, Entity obstacle) {
		// Apply damage to the player
		Health& playerHealth = registry.healths.get(player);
		playerHealth.current_health -= 10; // Example damage value
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
		} else {
			// Bounce back functionality by moving back by a bit
			Motion& playerMotion = registry.motions.get(player);
			playerMotion.velocity *= -1;
			playerMotion.position += playerMotion.velocity * 0.1f;
		}
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

	// TODO: Change to fire until clip is empty then auto reload
	if (button == left_click) {
		if (action == press) {
			// player is firing 
			registry.players.get(player).is_firing = true;
		}
		else if (action == release) {
			registry.players.get(player).is_firing = false;
			registry.players.get(player).fire_length_ms = 0.0f;
		}
	}
}