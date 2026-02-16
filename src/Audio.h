#pragma once

#include <cstring>
#include <SDL3/SDL.h>

struct SoundEffect {
    Uint8* buffer = nullptr;
    Uint32 length = 0;
    SDL_AudioSpec spec;
    SDL_AudioStream* stream = nullptr;

    bool Load(const char* filename) {
        // Load the WAV file into memory
        if (!SDL_LoadWAV(filename, &spec, &buffer, &length)) {
            return false;
        }
        // Create a stream that matches the file's spec and outputs to the default device
        stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, NULL, NULL);
        if (!stream) return false;

        SDL_ResumeAudioStreamDevice(stream);
        return true;
    }

    void Play() {
        if (stream) {
            // Clear any lingering data and queue the sound
            SDL_ClearAudioStream(stream); 
            SDL_PutAudioStreamData(stream, buffer, length);
        }
    }
};