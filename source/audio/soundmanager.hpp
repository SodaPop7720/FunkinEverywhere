#pragma once
#include <vector>

class FunkinSound;

class SoundManager
{
    private:
        std::vector<FunkinSound*> dasounds;

    public:
        void init();
        
        void add(FunkinSound* sound);
        void remove(FunkinSound* sound);
        void clear();

        void update(float elapsed);

        float current_position_seconds = 0.0f;
};

extern SoundManager sounds;