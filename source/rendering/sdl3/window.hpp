#pragma once
#include <SDL3/SDL.h>
#include <string>
#include <vector>

#include "../window.hpp"
#include "../../graphics/sdl3/sdl3sprite.hpp"

class WindowSDL3 : public WindowFE 
{
    public:
        bool debugMode;
        bool running;
      
        bool create(int width, int height, const std::string &title);
        float calcElapsed();
        void destroy();
        SDL_Window* getWindow();
        bool shouldClose();
        void pollEvents();
        void swapBuffers();
        void resize(int width, int height);

        int getWidth() const;
        int getHeight() const;
        float getPixelDensity() const;
        void *getHandle();
        
        float renderScale;
        bool hasFrameBegan;

        bool checkFramerate();
        void setRenderScale();
        void* getRenderer() override;

        void beginFrame(int screen, int colorR, int colorG, int colorB);
        void endFrame(bool shouldFlush = true);
        
        SDL_Window *window = nullptr;
        SDL_Renderer *renderer = nullptr;
        SDL_GLContext context = nullptr;

    private:
        int width = 0;
        int height = 0;
        float pixelDensity = 1.0f;
        bool shouldCloseFlag = false;
        
        Uint64 lastTime = SDL_GetTicks();
};
