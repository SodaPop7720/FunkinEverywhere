#pragma once
#include <SDL3/SDL.h>
#include <memory>
#include <string>
#include <vector>

#include "../basesound.hpp"

class SDL3SoundFE : public BaseSound
{
    public:
        SDL3SoundFE(const std::string& path, const std::string& type = "none");
        void create(const std::string& path, const std::string& type = "none") override;
        ~SDL3SoundFE() override;

        void play() override;
        void pause() override;

        void playMusic() override;
        void pauseMusic() override;
        void stopMusic() override;

        void update(float elapsed) override;
    private:
        SDL_AudioStream* stream = nullptr;

        int bytes_per_second = 0;
        Uint64 total_bytes_pushed = 0;
};