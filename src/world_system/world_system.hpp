#pragma once

// internal
#include "common/common.hpp"

// stlib
#include <vector>
#include <random>

#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_mixer.h>

#include "render_system/render_system.hpp"

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
	void init(RenderSystem* renderer);

	// Releases all associated resources
	~WorldSystem();

	// Steps the game ahead by ms milliseconds
	bool step(float elapsed_ms);

	// Check for collisions
	void handle_collisions();

	void cycle_weapon(int direction);

	void bounce_back(Entity player, Entity obstacle);

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

	//fps
	void fpsCalculate();
	// OpenGL window handle
	GLFWwindow* window;

	// Number of bug eaten by the chicken, displayed in the window title
	//unsigned int points;

	// Game state
	RenderSystem* renderer;
	// TODO: Global game state goes here
	float current_speed;
	Entity player;
	
	// HUD
	Entity player_hp_text;
	Entity weapon_text;
	Entity ammo_text;
	Entity score_text;
	int score;

	// FPS 
	Entity fps_text;
	float fps;
	float maxFps;
	float frameTime;

	// music references
	// TODO: ADD OUR SOUND REFERENCES AS Mix_Music* or Mix_Chunk* 


	// C++ random number generator
	std::default_random_engine rng;
	std::uniform_real_distribution<float> uniform_dist; // number between 0..1
};
