#pragma once

#include <cstring>
#include <SDL3/SDL.h>

struct SoundEffect {
    Uint8* buffer = nullptr;
    Uint32 length = 0;
    SDL_AudioSpec spec;
    SDL_AudioStream* stream = nullptr;
};