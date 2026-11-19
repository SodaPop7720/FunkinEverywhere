#include "titlestate.hpp"

TitleState::TitleState()
{
    create();
}

TitleState::~TitleState()
{
    destroy();
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

    freakyMenu = new FunkinSound("ui/main-menu/freaky-menu/freaky-menu", "music");
    sounds.add(freakyMenu);

    confirmSound = new FunkinSound("ui/main-menu/confirm-menu", "sound");
    sounds.add(confirmSound);
}

void TitleState::create()
{
    startIntro();
    conductor.setBPM(102);
    freakyMenu -> playMusic();
    isInitialized = true;
}

void TitleState::update(float elapsed)
{
    if(inputManager.justPressed(FUNKIN_ACTION::ACCEPT))
    {
        titleText -> playAnim("press");
        confirmSound -> play();
    }
}

void TitleState::destroy()
{

}

void TitleState::stepHit(int step)
{
    
}

void TitleState::beatHit(int beat)
{
    logoBl -> playAnim("bump");
    if (beat % 2 == 0) gfDance -> playAnim("danceLeft"); else gfDance -> playAnim("danceRight");
}

void TitleState::measureHit(int measure)
{
    
}