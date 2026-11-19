#include "basestate.hpp"

BaseState::~BaseState() = default;

BaseState *StateManager::curState = nullptr;
BaseState *StateManager::prevState = nullptr;

StateManager states;

void StateManager::changeState(BaseState *state)
{
    if (curState != nullptr) curState -> destroy();

    if (prevState != nullptr && prevState != state)
    {
        delete prevState;
        prevState = nullptr;
    }

    if (state != nullptr)
    {
        prevState = curState;
        curState = state;
        if (!curState -> isInitialized) curState -> create();
    }
    else
    {
        curState = nullptr;
    }
}

void StateManager::update(float elapsed)
{
    if (curState && curState != nullptr)
    {
        curState -> update(elapsed);
    }
}

void StateManager::destroy()
{
    if (curState != nullptr)
    {
        curState -> destroy();
        delete curState;
        curState = nullptr;
    }
    if (prevState != nullptr)
    {
        delete prevState;
        prevState = nullptr;
    }
}

void StateManager::stepHit(int step)
{
    if (curState && curState != nullptr) curState -> stepHit(step);
}

void StateManager::beatHit(int beat)
{
    if (curState && curState != nullptr) curState -> beatHit(beat);
}

void StateManager::measureHit(int measure)
{
    if (curState && curState != nullptr) curState -> measureHit(measure);
}