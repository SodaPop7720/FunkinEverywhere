#include <SDL3/SDL.h>
#include <iostream>

#include "../../input/inputmanager.hpp"
#include "../../input/sdl3/sdl3input.hpp"
#include "window.hpp"

bool WindowSDL3::create(int width, int height, const std::string &title)
{
    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return false;
    }

    if (!SDL_Init(SDL_INIT_GAMEPAD))
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
    SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);

    this -> width = width;
    this -> height = height;
    this -> pixelDensity = SDL_GetWindowPixelDensity(window);

    SDL_SetRenderLogicalPresentation(renderer, width, height, SDL_LOGICAL_PRESENTATION_LETTERBOX);
    SDL_SetRenderVSync(renderer, 1);
    
    initInput();

    return true;
}

float WindowSDL3::calcElapsed()
{
    Uint64 currentTime = SDL_GetTicks();
    float elapsed = (currentTime - lastTime) / 1000.0f;
    lastTime = currentTime;

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
{
    running = !shouldCloseFlag;

    SDL_SetRenderDrawColor(renderer, colorR, colorG, colorB, 255);
    SDL_RenderClear(renderer);

    hasFrameBegan = true;
}

void WindowSDL3::endFrame(bool shouldFlush)
{
    SDL_RenderPresent(renderer);

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
    inputManager.update();

    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        keyboardHandler.handleEvent(event, inputManager);
        controllerHandler.handleEvent(event, inputManager);
        touchHandler.handleEvent(event, inputManager);

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