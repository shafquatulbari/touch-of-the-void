#pragma once

// internal
#include "audio_manager/audio_manager.hpp"
#include "common/common.hpp"
#include "render_system/render_system.hpp"
#include "ui_system/ui_system.hpp"
#include "weapon_system/weapon_system.hpp"

// stlib
#include <vector>
#include <random>

// Container for all our entities and game logic. Individual rendering / update is
// deferred to the relative update() methods
class WorldSystem
{
public:
	WorldSystem();

	float restart_delay_timer = 0.0f; // Delay before restarting the game after death

	// Creates a window
	GLFWwindow* create_window();

	// starts the game
	void init(RenderSystem* renderer_arg, UISystem* ui_arg, WeaponSystem* weapon_arg);

	// Releases all associated resources
	~WorldSystem();

	// Steps the game ahead by ms milliseconds
	bool step(float elapsed_ms);

	// Check for collisions
	void handle_collisions();

	void bounce_back(Entity player, Entity obstacle, float scalar);

	// Should the game be over ?
	bool is_over()const;
private:
	// Input callback functions
	void on_key(int key, int, int action, int mod);
	void on_mouse_move(vec2 pos);
	void on_mouse_click(int button, int action, int mod);

	// restart level
	void restart_game();

	// enter room
	void enter_room(Room& room, vec2 player_pos);

	// fps
	void fpsCalculate();

	// OpenGL window handle
	GLFWwindow* window;

	// Game state
	RenderSystem* renderer;
	UISystem* ui;
	WeaponSystem* weapons;
	Entity player;

	std::array<Entity, 10> p_mesh_lines; // for debug

	enum class GAME_STATE {
		START_MENU,
		GAME,
		GAME_OVER,
		TOTAL_GAME_STATES // Keep this as the last element
	};

	GAME_STATE game_state = GAME_STATE::START_MENU;
	
	// HUD
	// Entity player_hp_text;
	// Entity weapon_text;
	// Entity ammo_text;
	Entity score_text;
	int score;
	float multiplier; // Min 0.0, Max 9.9
	int delta_score; // Score gained in the current period TBD what a period is

	// FPS 
	Entity fps_text;
	float fps;
	float maxFps;
	float frameTime;

	// C++ random number generator
	std::default_random_engine rng;
	std::uniform_real_distribution<float> uniform_dist; // number between 0..1
};
