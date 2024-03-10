#pragma once

#include "common/common.hpp"

#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_mixer.h>
#include <string>

// Function to initialize SDL audio
bool init_audio();

// Function to play a sound
void play_sound(Mix_Chunk* sound);

// Function to clean up and close audio
void close_audio();

// Sound references
extern Mix_Chunk* machine_gun_sound;
extern Mix_Chunk* sniper_sound;
extern Mix_Chunk* shotgun_sound;
extern Mix_Chunk* reload_start_sound;
extern Mix_Chunk* reload_end_sound;
extern Mix_Chunk* explosion_sound;
extern Mix_Chunk* cycle_weapon_sound;
extern Mix_Chunk* player_hit_sound;
extern Mix_Chunk* enemy_hit_sound;
