#include "audio_manager/audio_manager.hpp"

Mix_Chunk* machine_gun_sound = nullptr;
Mix_Chunk* sniper_sound = nullptr;
Mix_Chunk* shotgun_sound = nullptr;
Mix_Chunk* reload_start_sound = nullptr;
Mix_Chunk* reload_end_sound = nullptr;
Mix_Chunk* explosion_sound = nullptr;
Mix_Chunk* cycle_weapon_sound = nullptr;
Mix_Chunk* player_hit_sound = nullptr;
Mix_Chunk* enemy_hit_sound = nullptr;

bool init_audio() {
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
    machine_gun_sound = Mix_LoadWAV(audio_path("machine_gun.wav").c_str());
    sniper_sound = Mix_LoadWAV(audio_path("sniper.wav").c_str());
    shotgun_sound = Mix_LoadWAV(audio_path("shotgun.wav").c_str());
    reload_start_sound = Mix_LoadWAV(audio_path("reload_start_sound.wav").c_str());
    reload_end_sound = Mix_LoadWAV(audio_path("reload_end_sound.wav").c_str());
    explosion_sound = Mix_LoadWAV(audio_path("explosion.wav").c_str());
    cycle_weapon_sound = Mix_LoadWAV(audio_path("cycle_weapon_sound.wav").c_str());
    player_hit_sound = Mix_LoadWAV(audio_path("player_hit_sound.wav").c_str());
    enemy_hit_sound = Mix_LoadWAV(audio_path("enemy_hit_sound.wav").c_str());

    if (machine_gun_sound == nullptr ||
        sniper_sound == nullptr ||
        shotgun_sound == nullptr ||
        reload_start_sound == nullptr ||
        reload_end_sound == nullptr ||
        explosion_sound == nullptr ||
        cycle_weapon_sound == nullptr ||
        player_hit_sound == nullptr ||
        enemy_hit_sound == nullptr) {
        fprintf(stderr, "Failed to load sounds make sure the data directory is present");
        return nullptr;
    }

    return true;
}

void play_sound(Mix_Chunk* sound) {
    if (sound != nullptr) {
        Mix_PlayChannel(-1, sound, 0);
    }
}

void close_audio() {
    if (machine_gun_sound != nullptr)
        Mix_FreeChunk(machine_gun_sound);
    if (sniper_sound != nullptr)
        Mix_FreeChunk(sniper_sound);
    if (shotgun_sound != nullptr)
        Mix_FreeChunk(shotgun_sound);
    if (reload_start_sound != nullptr)
        Mix_FreeChunk(reload_start_sound);
    if (reload_end_sound != nullptr)
        Mix_FreeChunk(reload_end_sound);
    if (explosion_sound != nullptr)
        Mix_FreeChunk(explosion_sound);
    if (cycle_weapon_sound != nullptr)
        Mix_FreeChunk(cycle_weapon_sound);
    if (player_hit_sound != nullptr)
        Mix_FreeChunk(player_hit_sound);
    if (enemy_hit_sound != nullptr)
        Mix_FreeChunk(enemy_hit_sound);
}
