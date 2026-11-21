#include <algorithm>

#include "mainmenustate.hpp"

MainMenuState::MainMenuState()
{
    create();
}

MainMenuState::~MainMenuState()
{
    destroy();
}

void MainMenuState::create()
{
    bg = new FunkinSprite("ui/main-menu/menu-bg", "none");
    sprites.add(bg);
    
    menuItems = new FunkinSprite("ui/main-menu/items/story-mode", "sparrow");
    menuItems -> setRenderSelf(false);
    sprites.add(menuItems);

    for (size_t i = 0; i < optionShit.size(); ++i)
    {
        auto menuItem = std::make_unique<FunkinSprite>("ui/main-menu/items/" + optionShit[i], "sparrow");

        menuItem -> addAnim("idle", optionShit[i] + " idle", 24, true);
        menuItem -> addAnim("selected", optionShit[i] + " selected", 24, true);
        menuItem -> playAnim("idle");

        menuItem -> getParams().x = (globalWindow -> getWidth() * 0.5) - (menuItem -> getWidth() / 2);
        menuItem -> getParams().y = ((globalWindow -> getHeight() - (160 * optionShit.size())) / 2) + i * 160;

        menuItems -> addChild(std::move(menuItem));
    }
}

void MainMenuState::update(float elapsed)
{

}

void MainMenuState::destroy()
{

}

void MainMenuState::stepHit(int step)
{
    
}

void MainMenuState::beatHit(int beat)
{
    
}

void MainMenuState::measureHit(int measure)
{
    
}