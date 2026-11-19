#include <SDL3/SDL.h>
#include <string>
#include <iostream>

#include <dr_mp3.h>
#include <dr_wav.h>
#include <stb_vorbis.c>

#include "sdl3sound.hpp"
#include "../soundmanager.hpp"
#include "../../util/coolutil.hpp"
#include "../../rendering/window.hpp"

void SDL3SoundFE::create(const std::string& path, const std::string& type)
{
    auto mods = listDirectory("mods");
    modName = (mods && !mods -> empty()) ? (*mods)[0] : "none";
    std::string assetPath = fileExists("mods/" + modName + "/" + path + ".ogg") ? "mods/" + modName + "/" : "assets/";

    int channels;
    int sample_rate;
    short* decoded_output = NULL;
    
    int total_samples = stb_vorbis_decode_filename((assetPath + path + ".ogg").c_str(), &channels, &sample_rate, &decoded_output);
    int buffer_size_bytes = total_samples * channels * sizeof(short);

    SDL_AudioSpec src_spec;
    src_spec.format = SDL_AUDIO_S16;
    src_spec.channels = channels;
    src_spec.freq = sample_rate;

    bytes_per_second = src_spec.freq * src_spec.channels * SDL_AUDIO_BYTESIZE(src_spec.format);
    total_bytes_pushed = buffer_size_bytes;

    stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &src_spec, NULL, NULL);
    SDL_PutAudioStreamData(stream, decoded_output, buffer_size_bytes);
}

SDL3SoundFE::SDL3SoundFE(const std::string& path, const std::string& type)
{
    create(path, type);
    soundType = type;
}

SDL3SoundFE::~SDL3SoundFE()
{
    if (stream == NULL) return;
    SDL_DestroyAudioStream(stream);
}

void SDL3SoundFE::play()
{
    if (stream == NULL) return;
    SDL_ResumeAudioStreamDevice(stream);
}

void SDL3SoundFE::pause()
{
    if (stream == NULL) return;
    SDL_PauseAudioStreamDevice(stream);
}

void SDL3SoundFE::playMusic()
{
    if (stream == NULL) return;
    SDL_ResumeAudioStreamDevice(stream);
}

void SDL3SoundFE::pauseMusic()
{
    if (stream == NULL) return;
    SDL_PauseAudioStreamDevice(stream);
}

void SDL3SoundFE::stopMusic()
{
    if (stream == NULL) return;
    SDL_ClearAudioStream(stream);
}

void SDL3SoundFE::update(float elapsed)
{
    if (!stream || soundType != "music") return;

    Uint64 queued = SDL_GetAudioStreamQueued(stream);
    Uint64 bytes_played = total_bytes_pushed > queued ? total_bytes_pushed - queued : 0;

    sounds.current_position_seconds = (double)bytes_played / bytes_per_second;
}