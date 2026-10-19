#pragma once
#include <vector>
#include <SDL3/SDL.h>
#include <string>
#include <iostream>
class FunkinSprite;
//NOW SPRITEMANAGER HANDLES THE RENDERING, INSTEAD OF WINDOW.cpp and hpp (because renderer was taking over the window and gpu didn't draw .m.
class SpriteManager
{
    private:
        std::vector<FunkinSprite*> dasprites;

    public:
        void add(FunkinSprite* sprite);
        void remove(FunkinSprite* sprite);
        void clear();
        
        void update(float elapsed);
        void render();
};
void setupGPUrender(SDL_GPUDevice* dev, SDL_Window* wind);//this just claims the window and a gpu device/or creates a new one if you jsut choose NULL but if 
//ya reading the code or somethin n u wanna make sdl3 shader lib or smth this would make it easier and just make the people need create gpudevice and a window
//prolly add to gpu device vector instead of just switching idk
void setupspritehandlershader();
void loadGraphic(const std::string& path, SDL_GPUTexture*& texture, int& sizex, int& sizey);
extern SpriteManager sprites;
