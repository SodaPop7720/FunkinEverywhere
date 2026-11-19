#include <algorithm>

#include "spritemanager.hpp"
#include "basesprite.hpp"

SpriteManager sprites;

void SpriteManager::add(FunkinSprite* sprite)
{
    dasprites.push_back(sprite);
}

void SpriteManager::remove(FunkinSprite* sprite)
{
    dasprites.erase(std::remove(dasprites.begin(), dasprites.end(), sprite), dasprites.end());
}

void SpriteManager::add(FunkinGraphic* sprite)
{
    dagraphics.push_back(sprite);
}

void SpriteManager::remove(FunkinGraphic* sprite)
{
    dagraphics.erase(std::remove(dagraphics.begin(), dagraphics.end(), sprite), dagraphics.end());
}

void SpriteManager::clear()
{
    for (FunkinSprite* sprite : dasprites) delete sprite;
    dasprites.clear();
    for (FunkinGraphic* sprite : dagraphics) delete sprite;
    dagraphics.clear();
}

void SpriteManager::update(float elapsed)
{
    for (FunkinSprite* sprite : dasprites)
    {
        sprite -> update(elapsed);
    }
    for (FunkinGraphic* sprite : dagraphics)
    {
        sprite -> update(elapsed);
    }
}

void SpriteManager::render()
{
    for (FunkinSprite* sprite : dasprites)
    {
        sprite -> render();
    }
    for (FunkinGraphic* sprite : dagraphics)
    {
        sprite -> render();
    }
}