#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include <cstdlib>
#include <ctime>

#include "audio/soundmanager.hpp"
#include "graphics/spritemanager.hpp"
#include "rendering/sdl3/window.hpp"
#include "funkin/states/titlestate.hpp"
#include "funkin/song/conductor.hpp"
#include "input/inputmanager.hpp"
#include "util/coolutil.hpp"

WindowFE *globalWindow = nullptr;
std::string modName;

int windowWidth = 1280;
int windowHeight = 720;

int main(int argc, char **argv) 
{
    globalWindow = new WindowSDL3();
    if (!globalWindow -> create(windowWidth, windowHeight, "Funkin' Everywhere")) 
    {
        delete globalWindow;
        globalWindow = nullptr;
        return 1;
    }
    sounds.init();

    std::srand(static_cast<unsigned int>(std::time(0))); // randomness is done through this to support older c++ versions

    TitleState *initialState = new TitleState(); // replace this line if you wanna have a different initial state
    states.changeState(initialState);

    while(!globalWindow -> shouldClose())
    {
        float elapsed = globalWindow -> calcElapsed();

        globalWindow -> pollEvents();
        sprites.update(elapsed);
        sounds.update(elapsed);
        conductor.updateBeats(elapsed);
        states.update(elapsed);
        globalWindow -> beginFrame(0, 0, 0, 0);
        sprites.render();
        globalWindow -> endFrame(true);
    }

    sprites.clear();
    globalWindow -> destroy();

    return 0;
}