#include <iostream>
#include <cmath>

#include "conductor.hpp"
#include "../states/basestate.hpp"

Conductor conductor;

void Conductor::setBPM(float newbpm)
{
    swagTimer = 0.0f;
    curStepFloat = 0.0f;

    bpm = newbpm;

    if (bpm <= 10.0f) bpm = 10.0f;

    curStep = 0;
    curBeat = 0;
    curMeasure = 0;

    prevcurStep = 0;
    prevcurBeat = 0;
    prevcurMeasure = 0;
}

void Conductor::changeBPM(float newbpm)
{

}

void Conductor::updateBeats(float elapsed)
{
    swagTimer += elapsed;
    curStepFloat = (swagTimer / (15000 / bpm) * 1000);

    curStep = std::floor(curStepFloat);
    curBeat = std::floor(curStepFloat / stepsPerBeat);
    curMeasure = std::floor(curStepFloat / stepsPerMeasure);

    if (curStep != prevcurStep)
    {
        for (int i = prevcurStep + 1; i <= curStep; i++) onStepHit(i);
    }
    if (curBeat != prevcurBeat)
    {
        for (int i = prevcurBeat + 1; i <= curBeat; i++) onBeatHit(i);
    }
    if (curMeasure != prevcurMeasure)
    {
        for (int i = prevcurMeasure + 1; i <= curMeasure; i++) onMeasureHit(i);
    }

    prevcurStep = curStep;
    prevcurBeat = curBeat;
    prevcurMeasure = curMeasure;
}

void Conductor::onStepHit(int dacurStep)
{
    StateManager::stepHit(dacurStep);
}

void Conductor::onBeatHit(int dacurBeat)
{
    StateManager::beatHit(dacurBeat);
}

void Conductor::onMeasureHit(int dacurMeasure)
{
    StateManager::measureHit(dacurMeasure);
}