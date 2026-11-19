#pragma once
#include <vector>

class FunkinSprite;
class FunkinGraphic;

class SpriteManager
{
    private:
        std::vector<FunkinSprite*> dasprites;
        std::vector<FunkinGraphic*> dagraphics;

    public:
        void add(FunkinSprite* sprite);
        void remove(FunkinSprite* sprite);

        void add(FunkinGraphic* sprite);
        void remove(FunkinGraphic* sprite);

        void clear();

        void update(float elapsed);
        void render();
};

extern SpriteManager sprites;