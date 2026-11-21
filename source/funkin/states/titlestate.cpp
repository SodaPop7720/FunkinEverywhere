#include "titlestate.hpp"
#include "mainmenustate.hpp"

#include <cstdlib>
#include <fstream>
#include <string>
#include <vector>

TitleState::TitleState()
{
    create();
}

TitleState::~TitleState()
{
    destroy();
}

std::vector<std::vector<std::string>> TitleState::getIntroTextShit()
{
    std::string assetPath = fileExists("mods/" + modName + "/ui/title/intro-text.txt") ? "mods/" + modName + "/" : "assets/";
    std::ifstream file((assetPath + "ui/title/intro-text.txt").c_str());

    std::vector<std::vector<std::string>> swagGoodArray;
    std::string line;

    while (std::getline(file, line))
    {
        if (line.empty()) continue;

        std::vector<std::string> firstArray;
        size_t pos = 0;

        while ((pos = line.find("--")) != std::string::npos)
        {
            firstArray.push_back(line.substr(0, pos));
            line.erase(0, pos + 2);
        }

        firstArray.push_back(line);
        swagGoodArray.push_back(firstArray);
    }

    return swagGoodArray;
}

void TitleState::startIntro()
{
    logoBl = new FunkinSprite("ui/title/logo-bumpin", "sparrow");
    logoBl -> getParams().x = -150;
    logoBl -> getParams().y = -100;
    logoBl -> addAnim("bump", "logo bumpin", 24, false);
    sprites.add(logoBl);
    
    gfDance = new FunkinSprite("ui/title/gf-dance-title", "sparrow");
    gfDance -> getParams().x = globalWindow -> getWidth() * 0.4;
    gfDance -> getParams().y = globalWindow -> getHeight() * 0.07;
    gfDance -> addAnim("danceLeft", "gfDance", {30, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14}, 24, true);
    gfDance -> addAnim("danceRight", "gfDance", {15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29}, 24, true);
    sprites.add(gfDance);
    
    titleText = new FunkinSprite("ui/title/title-screen-text", "animateatlas");
    titleText -> getParams().x = 100;
    titleText -> getParams().y = globalWindow -> getHeight() * 0.8;
    titleText -> addSpritemapAnim("idle", "Idle", {}, 24, true);
    titleText -> addSpritemapAnim("press", "Confirm", {}, 24, true);
    titleText -> playAnim("idle");
    sprites.add(titleText);

    logoBl -> getParams().visible = false;
    gfDance -> getParams().visible = false;
    titleText -> getParams().visible = false;

    ngSpr = new FunkinSprite("ui/title/newgrounds-logo");
    ngSpr -> getParams().x = (globalWindow -> getWidth() * 0.5) - (ngSpr -> getWidth() / 2);
    ngSpr -> getParams().y = (globalWindow -> getHeight() * 0.52) + 100;
    ngSpr -> getParams().scaleX = 0.8;
    ngSpr -> getParams().scaleY = 0.8;
    ngSpr -> getParams().visible = false;
    sprites.add(ngSpr);

    coolText1 = new AtlasText("", AtlasFont::BOLD, "ui/fonts/bold");
    coolText1 -> setPosition(0, 200);
    sprites.add(coolText1);

    coolText2 = new AtlasText("", AtlasFont::BOLD, "ui/fonts/bold");
    coolText2 -> setPosition(0, 200);
    sprites.add(coolText2);

    coolText3 = new AtlasText("", AtlasFont::BOLD, "ui/fonts/bold");
    coolText3 -> setPosition(0, 200);
    sprites.add(coolText3);

    whiteFlash = new FunkinGraphic(globalWindow -> getWidth(), globalWindow -> getHeight());
    whiteFlash -> getParams().alpha = 0;
    sprites.add(whiteFlash);

    freakyMenu = new FunkinSound("ui/main-menu/freaky-menu/freaky-menu", "music");
    sounds.add(freakyMenu);

    confirmSound = new FunkinSound("ui/main-menu/confirm-menu", "sound");
    sounds.add(confirmSound);
}

void TitleState::skipIntro()
{
    introSkipped = true;

    logoBl -> getParams().visible = true;
    gfDance -> getParams().visible = true;
    titleText -> getParams().visible = true;

    daFlashAlpha = 1;
}

void TitleState::create()
{
    introSkipped = false;
    enterPressed = false;

    auto introText = getIntroTextShit();

    if (!introText.empty())
    {
        curWacky = introText[std::rand() % (introText.size() - 1)];
    }

    startIntro();
    freakyMenu -> playMusic();
    conductor.setBPM(102);
    isInitialized = true;
}

void TitleState::goToMenu()
{
    MainMenuState *mainmenustate = new MainMenuState();
    states.changeState(mainmenustate);
}

void TitleState::update(float elapsed)
{
    if(daFlashAlpha > 0) daFlashAlpha -= elapsed / (enterPressed ? 1 : 2); else daFlashAlpha = 0;

    whiteFlash -> getParams().alpha = daFlashAlpha;

    if(introSkipped)
    {
        coolText1 -> setAlpha(0);
        coolText2 -> setAlpha(0);
        coolText3 -> setAlpha(0);
        ngSpr -> getParams().visible = false;
    }

    if(inputManager.justPressed(FUNKIN_ACTION::ACCEPT))
    {
        if(introSkipped)
        {
            if(enterPressed)
            {
                goToMenu();
            }
            else
            {
                enterPressed = true;
                daFlashAlpha = 1;

                titleText -> playAnim("press");
                confirmSound -> play();
            }
        }
        else
        {
            skipIntro();
        }
    }

    if(enterPressed)
    {
        timeTillMenu += elapsed;
        if(timeTillMenu >= 2)
        {
            goToMenu();
        }
    }
}

void TitleState::destroy()
{
    sprites.clear();
}

void TitleState::stepHit(int step)
{
    
}

void TitleState::beatHit(int beat)
{
    logoBl -> playAnim("bump");
    if (beat % 2 == 0) gfDance -> playAnim("danceLeft"); else gfDance -> playAnim("danceRight");

    if(!introSkipped)
    {
        switch (beat)
        {
            case 1:
                coolText1 -> setText("The");
                coolText1 -> setPosition((globalWindow -> getWidth() * 0.5) - (coolText1 -> getWidth() / 2), 200);
                coolText2 -> setText("Funkin' Crew Inc");
                coolText2 -> setPosition((globalWindow -> getWidth() * 0.5) - (coolText2 -> getWidth() / 2), 260);
                break;
            case 3:
                coolText3 -> setText("Presents");
                coolText3 -> setPosition((globalWindow -> getWidth() * 0.5) - (coolText3 -> getWidth() / 2), 320);
                break;
            case 4:
                coolText1 -> setText("");
                coolText2 -> setText("");
                coolText3 -> setText("");
                break;
            case 5:
                coolText1 -> setText("In association");
                coolText1 -> setPosition((globalWindow -> getWidth() * 0.5) - (coolText1 -> getWidth() / 2), 200);
                coolText2 -> setText("With");
                coolText2 -> setPosition((globalWindow -> getWidth() * 0.5) - (coolText2 -> getWidth() / 2), 260);
                break;
            case 7:
                coolText3 -> setText("Newgrounds");
                coolText3 -> setPosition((globalWindow -> getWidth() * 0.5) - (coolText3 -> getWidth() / 2), 320);
                ngSpr -> getParams().visible = true;
                break;
            case 8:
                coolText1 -> setText("");
                coolText2 -> setText("");
                coolText3 -> setText("");
                ngSpr -> getParams().visible = false;
                break;
            case 9:
                coolText1 -> setText(curWacky.at(0));
                coolText1 -> setPosition((globalWindow -> getWidth() * 0.5) - (coolText1 -> getWidth() / 2), 200);
                break;
            case 11:
                coolText2 -> setText(curWacky.at(1));
                coolText2 -> setPosition((globalWindow -> getWidth() * 0.5) - (coolText2 -> getWidth() / 2), 260);
                break;
            case 12:
                coolText1 -> setText("");
                coolText2 -> setText("");
                coolText3 -> setText("");
                break;
            case 13:
                coolText1 -> setText("Friday");
                coolText1 -> setPosition((globalWindow -> getWidth() * 0.5) - (coolText1 -> getWidth() / 2), 200);
                break;
            case 14:
                coolText2 -> setText("Night");
                coolText2 -> setPosition((globalWindow -> getWidth() * 0.5) - (coolText2 -> getWidth() / 2), 260);
                break;
            case 15:
                coolText3 -> setText("Funkin'");
                coolText3 -> setPosition((globalWindow -> getWidth() * 0.5) - (coolText3 -> getWidth() / 2), 320);
                break;
            case 16:
                skipIntro();
                break;
        }
    }
}

void TitleState::measureHit(int measure)
{
    
}