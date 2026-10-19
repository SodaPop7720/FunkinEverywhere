#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <string>
#include <iostream>

#include "../../rendering/sdl3/window.hpp"
#include "../../rendering/window.hpp"
#include "sdl3sprite.hpp"
#include "../cppanimate.hpp"
#include "../../util/coolutil.hpp"
#include "../spritemanager.hpp"

void SDL3Sprite::create(const std::string& path, const std::string& type)
{
    auto mods = listDirectory("mods");
    modName = (mods && !mods->empty()) ? (*mods)[0] : "none";

    if (type != "animateatlas" && type != "multianimate")
    {
        std::string assetPath = fileExists("mods/" + modName + "/" + path + ".png") ? "mods/" + modName + "/" : "mods/" + modName + "/" + "assets/";
        texture = IMG_LoadTexture(static_cast<SDL_Renderer*>(globalWindow->getRenderer()), (assetPath + path + ".png").c_str());
        if (type == "sparrow") 
            loadAtlas(assetPath + path + ".xml");
        loadGraphic(assetPath + path + ".png", gputext, sizex, sizey);
        if (!gputext) {
            std::cout << "TEXTURE IS WRONG SINCE HERE!";
        }
        std::cout << "\n\n Texture size of " + path + ".png: " << "\n\n x:" << sizex << "\n\n y:"  << sizey;
        
    }
   
    SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_NEAREST);
}
SDL_GPUTexture* SDL3Sprite::gettexture() {
    return gputext;
}
SDL3Sprite::SDL3Sprite(const std::string& path, const std::string& type)
{
    spriteType = type;
    create(path, type);
}


spritedata SDL3Sprite::getspritedata()
{
    float texWidth, texHeight;
    SDL_GetTextureSize(texture, &texWidth, &texHeight);


    if (spriteType == "sparrow")
    {
        AtlasFrame& frame = anims[curAnim].frames[curFrame];

        srect.x = frame.x;
        srect.y = frame.y;
        srect.w = frame.width;
        srect.h = frame.height;

        frameX = frame.frameX;
        frameY = frame.frameY;
        frameWidth = frame.frameWidth;
        frameHeight = frame.frameHeight;

        stuff.width = frameWidth;
        stuff.height = frameHeight;

    }
    else
    {
        srect.x = 0;
        srect.y = 0;
        srect.w = texWidth;
        srect.h = texHeight;

        frameX = 0;
        frameY = 0;
        stuff.width = frameWidth = texWidth;
        stuff.height = frameHeight = texHeight;

    }

    drect.x = (stuff.x - frameX) * stuff.scaleX;
    drect.y = (stuff.y - frameY) * stuff.scaleY;
    drect.w = srect.w * stuff.scaleX;
    drect.h = srect.h * stuff.scaleY;

    float u0 = (srect.x / texWidth);
    float v0 = 1 - ((srect.y + frameHeight) / texHeight); //frameHeight is the only constant  i can see that don't vary drastically with piexel height n shit 

    float u1 = (srect.w) / texWidth;
    float v1 = (srect.h*1.01) / texHeight;
    if (!gputext) {
        std::cout << "MANGLED FROM ORIGIN.";
    }
    
    spritedata spritedata;
    spritedata.s_uv = { u0, v0 };  
    spritedata.s_size = { u1, v1 };
    spritedata.scalex = drect.w;
    spritedata.scaley = drect.h;
    spritedata.xpos = drect.x;
    
    spritedata.ypos = -drect.y + stuff.skewY; //miracle. i should prolly debug it more
    std::cout << "\n\n" << curFrame << "\n\n";
    return spritedata;
}

SDL3Sprite::~SDL3Sprite()
{
    SDL_DestroyTexture(texture);
}
