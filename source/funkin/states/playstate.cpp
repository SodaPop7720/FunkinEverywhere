#include "playstate.hpp"

PlayState::PlayState()
{
    create();
}

PlayState::~PlayState()
{
    destroy();
}

void PlayState::create()
{
    
}

void PlayState::update(float elapsed)
{
    
}

void PlayState::destroy()
{
    sprites.clear();
    sounds.clear();
}

void PlayState::stepHit(int step)
{
    
}

void PlayState::beatHit(int beat)
{
    
}

void PlayState::measureHit(int measure)
{
    
}