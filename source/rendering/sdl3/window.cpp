#include <SDL3/SDL.h>
#include <iostream>

#include "window.hpp"

SDL_Window* WindowSDL3::getWindow() {
    return window;
}
bool WindowSDL3::create(int width, int height, const std::string &title)
{
    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return false;
    }
   
    if (!SDL_CreateWindowAndRenderer(title.c_str(), width, height, SDL_WINDOW_RESIZABLE, &window, &renderer))
    {
        std::cerr << "Window/Renderer Creation Error: " << SDL_GetError() << std::endl;

        SDL_Quit();
        return false;
    }

    this -> width = width;
    this -> height = height;
    this -> pixelDensity = SDL_GetWindowPixelDensity(window);

    SDL_SetRenderLogicalPresentation(renderer, width, height, SDL_LOGICAL_PRESENTATION_LETTERBOX);
    SDL_SetRenderVSync(renderer, 1);

    return true;
}

float WindowSDL3::calcElapsed()
{
    Uint64 currentTime = SDL_GetTicks();
    int framespersec = 1;
    float frametime = 0;
    while (frametime < (float)1 / (float)(framespersec)) {
        frametime += (float)SDL_GetTicks()/1000.0f;
   }
    float elapsed = (currentTime - lastTime) / 1000.0f;
    lastTime = currentTime;
    std::cout << elapsed;
    frametime = 0;
    
    return elapsed;
}

void WindowSDL3::destroy()
{
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

bool WindowSDL3::shouldClose()
{
    return shouldCloseFlag;
}

void WindowSDL3::resize(int width, int height)
{
    this -> width = width;
    this -> height = height;
    this -> pixelDensity = SDL_GetWindowPixelDensity(window);
}

int WindowSDL3::getWidth() const
{
    return width;
}

int WindowSDL3::getHeight() const
{
    return height;
}

float WindowSDL3::getPixelDensity() const
{
    return pixelDensity;
}

void *WindowSDL3::getHandle()
{
    return window;
}

void WindowSDL3::beginFrame(int screen, int colorR, int colorG, int colorB)
{//RENDERER IS GONE MUEHEHEHEHE
    running = !shouldCloseFlag;

    

    hasFrameBegan = true;
}

void WindowSDL3::endFrame(bool shouldFlush)
{//MUEHEHEHEHE
    

    hasFrameBegan = false;
}

void WindowSDL3::swapBuffers()
{
    return;
}

bool WindowSDL3::checkFramerate()
{
    return true;
}

void WindowSDL3::setRenderScale()
{
    return;
}

void* WindowSDL3::getRenderer()
{
    return renderer;
}

void WindowSDL3::pollEvents()
{
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) 
        {
            case SDL_EVENT_QUIT:
            {
                shouldCloseFlag = true;
            } break;
            case SDL_EVENT_WINDOW_RESIZED: 
            {
                int w, h;
                SDL_GetWindowSizeInPixels(window, &w, &h);
                this -> resize(w, h);
            } break;
        }
    }
}
