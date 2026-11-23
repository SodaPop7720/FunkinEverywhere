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

void MainMenuState::changeSelection(int silly)
{
    if (optionShit.empty()) return;

    curSelected += silly;
    if(curSelected < 0) curSelected = optionShit.size() - 1;
    if(curSelected >= optionShit.size()) curSelected = 0;

    for (size_t i = 0; i < menuItemSprites -> getChildren().size(); ++i)
    {
        auto* item = menuItemSprites -> getChildren()[i].get();
        if (!item) continue;

        item -> playAnim("idle");
        item -> getParams().x = (globalWindow -> getWidth() * 0.5) - (item -> getWidth() / 2);
        item -> getParams().y = ((globalWindow -> getHeight() - (160 * optionShit.size())) / 2) + i * 160;

        float coolX = item -> getParams().x;
        float coolY = item -> getParams().y;
        float coolWidth = item -> getWidth();
        float coolHeight = item -> getHeight();

        if(item -> getParams().id == curSelected)
        {
            item -> playAnim("selected");
            item -> getParams().x = coolX - ((item -> getWidth() - coolWidth) / 2);
            item -> getParams().y = coolY - ((item -> getHeight() - coolHeight) / 2);
        }
    }
}

void MainMenuState::create()
{
    bg = new FunkinSprite("ui/main-menu/menu-bg", "none");
    sprites.add(bg);
    
    menuItemSprites = new FunkinSprite("ui/main-menu/items/story-mode", "sparrow");
    menuItemSprites -> setRenderSelf(false);
    sprites.add(menuItemSprites);

    for (size_t i = 0; i < optionShit.size(); ++i)
    {
        auto menuItem = std::make_unique<FunkinSprite>("ui/main-menu/items/" + optionShit[i], "sparrow");

        menuItem -> addAnim("idle", optionShit[i] + " idle", 24, true);
        menuItem -> addAnim("selected", optionShit[i] + " selected", 24, true);
        menuItem -> playAnim("idle");

        menuItem -> getParams().x = (globalWindow -> getWidth() * 0.5) - (menuItem -> getWidth() / 2);
        menuItem -> getParams().y = ((globalWindow -> getHeight() - (160 * optionShit.size())) / 2) + i * 160;

        menuItem -> getParams().id = i;

        menuItems.push_back(menuItem.get());
        menuItemSprites -> addChild(std::move(menuItem));
    }

    changeSelection(0);
}

void MainMenuState::update(float elapsed)
{
    if(inputManager.justPressed(FUNKIN_ACTION::UI_UP))
    {
        changeSelection(-1);
    }
    if(inputManager.justPressed(FUNKIN_ACTION::UI_DOWN))
    {
        changeSelection(1);
    }
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