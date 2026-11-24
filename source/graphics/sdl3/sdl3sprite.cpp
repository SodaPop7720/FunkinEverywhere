#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <algorithm>
#include <cmath>
#include <string>
#include <iostream>

#include "../../rendering/sdl3/window.hpp"
#include "../../rendering/window.hpp"
#include "sdl3sprite.hpp"
#include "../../util/coolutil.hpp"
#include "../basesprite.hpp"

namespace
{
    AffineMatrix composeMatrix(const AffineMatrix& parent, const AffineMatrix& child)
    {
        AffineMatrix result;

        result.a = parent.a * child.a + parent.c * child.b;
        result.b = parent.b * child.a + parent.d * child.b;
        result.c = parent.a * child.c + parent.c * child.d;
        result.d = parent.b * child.c + parent.d * child.d;
        result.tx = parent.a * child.tx + parent.c * child.ty + parent.tx;
        result.ty = parent.b * child.tx + parent.d * child.ty + parent.ty;

        return result;
    }
}

static SpritemapStyle combineStyles(const SpritemapStyle& parent, const SpritemapStyle& child)
{
    SpritemapStyle result = parent;

    if (child.hasColor)
    {
        result.redOffset = parent.redOffset * child.redMultiplier + child.redOffset;
        result.greenOffset = parent.greenOffset * child.greenMultiplier + child.greenOffset;
        result.blueOffset = parent.blueOffset * child.blueMultiplier + child.blueOffset;
        result.alphaOffset = parent.alphaOffset * child.alphaMultiplier + child.alphaOffset;

        result.redMultiplier *= child.redMultiplier;
        result.greenMultiplier *= child.greenMultiplier;
        result.blueMultiplier *= child.blueMultiplier;
        result.alphaMultiplier *= child.alphaMultiplier;
    }

    if (child.hasBlend)
    {
        result.blend = child.blend;
        result.hasBlend = true;
    }

    result.hasColor = parent.hasColor || child.hasColor;
    return result;
}

static float applyColorTransform(float value, float multiplier, float offset)
{
    return std::clamp(value * multiplier + offset / 255.0f, 0.0f, 1.0f);
}

void SDL3Sprite::loadSprite(const std::string& path, const std::string& type)
{
    auto mods = listDirectory("mods");
    modName = (mods && !mods -> empty()) ? (*mods)[0] : "none";

	if(type != "animateatlas" && type != "multianimate") 
	{
        std::string assetPath = fileExists("mods/" + modName + "/" + path + ".png") ? "mods/" + modName + "/" : "assets/";
		texture = IMG_LoadTexture(static_cast<SDL_Renderer*>(globalWindow -> getRenderer()), (assetPath + path + ".png").c_str());
		if (type == "sparrow") loadAtlas(assetPath + path + ".xml");
	}
	else
	{
        std::string assetPath = fileExists("mods/" + modName + "/" + path + "/spritemap1.png") ? "mods/" + modName + "/" : "assets/";
		texture = IMG_LoadTexture(static_cast<SDL_Renderer*>(globalWindow -> getRenderer()), (assetPath + path + "/spritemap1.png").c_str());
		loadSpritemap(assetPath + path);
	}

	SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_NEAREST);
}

void SDL3Sprite::create(const std::string& path, const std::string& type)
{
    loadSprite(path, type);
}

SDL3Sprite::SDL3Sprite(const std::string& path, const std::string& type)
{
	spriteType = type;
	create(path, type);
}

void SDL3Sprite::render()
{
    if (!stuff.visible) return;
    BaseSprite::render();
    if (!renderSelf || !texture) return;

    if (spriteType == "animateatlas" || spriteType == "multianimate")
    {
        if (curAnim < 0 || curAnim >= static_cast<int>(anims.size())) return;

        const Animation& animation = anims[curAnim];
        if (animation.frames.empty() || curFrame < 0 || curFrame >= static_cast<int>(animation.frames.size())) return;

        renderSpritemap(animation.frames[curFrame].index);
        return;
    }

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

    float u0 = 0.0f;
    float u1 = 0.0f;
    float v0 = 0.0f;
    float v1 = 0.0f;

    if (stuff.flipX)
    {
        u0 = (srect.x + srect.w) / texWidth;
        u1 = srect.x / texWidth;
    }
    else
    {
        u0 = srect.x / texWidth;
        u1 = (srect.x + srect.w) / texWidth;
    }

    if (stuff.flipY)
    {
        v0 = (srect.y + srect.h) / texHeight;
        v1 = srect.y / texHeight;
    }
    else
    {
        v0 = srect.y / texHeight;
        v1 = (srect.y + srect.h) / texHeight;
    }

    float rad = stuff.angle * (std::acos(-1.0f) / 180.0f);
    float cosA = cosf(rad);
    float sinA = sinf(rad);

    float centerX = drect.x + (drect.w / 2.0f);
    float centerY = drect.y + (drect.h / 2.0f);

    SDL_FPoint p_tl = {(drect.x + (stuff.skewX / 2.0f)) - centerX, (drect.y + (stuff.skewY / 2.0f)) - centerY};
    SDL_FPoint p_tr = {((drect.x + drect.w) + (stuff.skewX / 2.0f)) - centerX, (drect.y - (stuff.skewY / 2.0f)) - centerY};
    SDL_FPoint p_br = {((drect.x + drect.w) - (stuff.skewX / 2.0f)) - centerX, ((drect.y + drect.h) - (stuff.skewY / 2.0f)) - centerY};
    SDL_FPoint p_bl = {(drect.x - (stuff.skewX / 2.0f)) - centerX, ((drect.y + drect.h) + (stuff.skewY / 2.0f)) - centerY};

    SDL_Vertex vertices[4] = {
        {{centerX + (p_tl.x * cosA - p_tl.y * sinA), centerY + (p_tl.x * sinA + p_tl.y * cosA)}, {stuff.color.r, stuff.color.g, stuff.color.b, stuff.alpha}, {u0, v0}},
        {{centerX + (p_tr.x * cosA - p_tr.y * sinA), centerY + (p_tr.x * sinA + p_tr.y * cosA)}, {stuff.color.r, stuff.color.g, stuff.color.b, stuff.alpha}, {u1, v0}},
        {{centerX + (p_br.x * cosA - p_br.y * sinA), centerY + (p_br.x * sinA + p_br.y * cosA)}, {stuff.color.r, stuff.color.g, stuff.color.b, stuff.alpha}, {u1, v1}},
        {{centerX + (p_bl.x * cosA - p_bl.y * sinA), centerY + (p_bl.x * sinA + p_bl.y * cosA)}, {stuff.color.r, stuff.color.g, stuff.color.b, stuff.alpha}, {u0, v1}}
    };

    int daindices[6] = {0, 1, 2, 2, 3, 0};

    SDL_RenderGeometry(static_cast<SDL_Renderer*>(globalWindow -> getRenderer()), texture, vertices, 4, daindices, 6);
}

void SDL3Sprite::visitSymbolPieces(const std::string& symbolName, int frame, const AffineMatrix& parentMatrix, const SpritemapStyle& parentStyle, int recursionDepth,
    const std::function<void(const std::string&, const SpritemapPiece&, const AffineMatrix&, const SpritemapStyle&)>& callback) const
{
    if (recursionDepth >= 32) return;

    auto symbolIt = spritemapData.symbols.find(symbolName);
    if (symbolIt == spritemapData.symbols.end()) return;

    const SpritemapSymbol& symbol = symbolIt -> second;

    for (int i = static_cast<int>(symbol.layers.size()) - 1; i >= 0; --i)
    {
        const SpritemapKeyframe* keyframe = symbol.getKeyframe(i, frame);
        if (!keyframe) continue;

        SpritemapStyle frameStyle = parentStyle;

        if (keyframe -> hasBlend)
        {
            frameStyle.blend = keyframe -> blend;
            frameStyle.hasBlend = true;
        }

        for (const SpritemapElement& element : keyframe -> elements)
        {
            if (element.isAtlasSprite)
            {
                const auto& sprite = element.asi;

                auto pieceIt = spritemapData.atlas.find(sprite.pieceName);
                if (pieceIt == spritemapData.atlas.end()) continue;

                const AffineMatrix matrix = composeMatrix(parentMatrix, sprite.mx);

                callback(sprite.pieceName, pieceIt -> second, matrix, frameStyle);
                continue;
            }

            const auto& instance = element.si;

            auto childIt = spritemapData.symbols.find(instance.symbolName);
            if (childIt == spritemapData.symbols.end()) continue;

            const SpritemapSymbol& child = childIt -> second;
            const int duration = child.duration();

            if (duration <= 0) continue;

            int childFrame = instance.firstFrame + (frame - keyframe->index);

            const bool loops = instance.loopType == "LOOP" || instance.loopType == "loop" || instance.loopType == "1";

            if (loops)
            {
                childFrame %= duration;
                if (childFrame < 0) childFrame += duration;
            }
            else
            {
                childFrame = std::clamp(childFrame, 0, duration - 1);
            }

            const AffineMatrix childMatrix = composeMatrix(parentMatrix, instance.mx);
            const SpritemapStyle childStyle = combineStyles(frameStyle, instance.style);

            visitSymbolPieces(instance.symbolName, childFrame, childMatrix, childStyle, recursionDepth + 1, callback);
        }
    }
}

void SDL3Sprite::renderSpritemap(int frame)
{
    if (spritemapData.rootSymbolName.empty()) return;

    auto rootIt = spritemapData.symbols.find(spritemapData.rootSymbolName);
    if (rootIt == spritemapData.symbols.end()) return;

    const int rootDuration = rootIt->second.duration();
    if (rootDuration <= 0) return;

    float minX = std::numeric_limits<float>::max();
    float minY = std::numeric_limits<float>::max();
    float maxX = std::numeric_limits<float>::lowest();
    float maxY = std::numeric_limits<float>::lowest();

    bool hasBounds = false;

    for (int i = 0; i < rootDuration; ++i)
    {
        visitSymbolPieces(spritemapData.rootSymbolName, i, AffineMatrix{}, SpritemapStyle{}, 0,
            [&](const std::string&, const SpritemapPiece& piece, const AffineMatrix& matrix, const SpritemapStyle&)
            {
                const float width = static_cast<float>(piece.rotated ? piece.h : piece.w);
                const float height = static_cast<float>(piece.rotated ? piece.w : piece.h);

                if (width <= 0.0f || height <= 0.0f) return;

                const float px[] = {0.0f, width, width, 0.0f};
                const float py[] = {0.0f, 0.0f, height, height};

                for (int j = 0; j < 4; ++j)
                {
                    const float x = matrix.a * px[j] + matrix.c * py[j] + matrix.tx;
                    const float y = matrix.b * px[j] + matrix.d * py[j] + matrix.ty;

                    minX = std::min(minX, x);
                    minY = std::min(minY, y);
                    maxX = std::max(maxX, x);
                    maxY = std::max(maxY, y);

                    hasBounds = true;
                }
            }
        );
    }

    if (!hasBounds) return;

    const float centerX = (minX + maxX) * 0.5f;
    const float centerY = (minY + maxY) * 0.5f;
    
    const float rad = std::acos(-1.0f) / 180.0f;

    const float rotation = stuff.angle * rad;
    const float skewX = std::tan(stuff.skewX * rad);
    const float skewY = std::tan(stuff.skewY * rad);

    const float cosR = std::cos(rotation);
    const float sinR = std::sin(rotation);

    AffineMatrix root;

    root.a = (cosR - sinR * skewY) * stuff.scaleX;
    root.b = (sinR + cosR * skewY) * stuff.scaleX;
    root.c = (cosR * skewX - sinR) * stuff.scaleY;
    root.d = (sinR * skewX + cosR) * stuff.scaleY;

    root.tx = stuff.x + (centerX - minX) - root.a * centerX - root.c * centerY;
    root.ty = stuff.y + (centerY - minY) - root.b * centerX - root.d * centerY;

    SDL_Renderer* renderer = static_cast<SDL_Renderer*>(globalWindow -> getRenderer());
    SDL_Texture* previousTarget = SDL_GetRenderTarget(renderer);

    int targetWidth = 0;
    int targetHeight = 0;

    if (previousTarget)
    {
        float w, h;
        SDL_GetTextureSize(previousTarget, &w, &h);

        targetWidth = static_cast<int>(w);
        targetHeight = static_cast<int>(h);
    }
    else
    {
        if (!SDL_GetCurrentRenderOutputSize(renderer, &targetWidth, &targetHeight)) return;
    }

    if (targetWidth <= 0 || targetHeight <= 0) return;

    if (!spritemapRenderTexture || spritemapRenderWidth != targetWidth || spritemapRenderHeight != targetHeight)
    {
        if (spritemapRenderTexture) SDL_DestroyTexture(spritemapRenderTexture);

        spritemapRenderTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, targetWidth, targetHeight);

        if (!spritemapRenderTexture)
        {
            spritemapRenderWidth = 0;
            spritemapRenderHeight = 0;
            SDL_Log("Failed to create spritemap render texture: %s", SDL_GetError());
            return;
        }

        spritemapRenderWidth = targetWidth;
        spritemapRenderHeight = targetHeight;

        SDL_SetTextureBlendMode(spritemapRenderTexture, SDL_BLENDMODE_BLEND_PREMULTIPLIED);
        SDL_SetTextureScaleMode(spritemapRenderTexture, SDL_SCALEMODE_NEAREST);
    }

    Uint8 oldR, oldG, oldB, oldA;
    SDL_GetRenderDrawColor(renderer, &oldR, &oldG, &oldB, &oldA);

    if (!SDL_SetRenderTarget(renderer, spritemapRenderTexture))
    {
        SDL_Log("Failed to set spritemap render target: %s", SDL_GetError());
        return;
    }

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);

    renderSymbol(spritemapData.rootSymbolName, frame, root, SpritemapStyle{}, 0);

    SDL_SetRenderTarget(renderer, previousTarget);
    SDL_SetRenderDrawColor(renderer, oldR, oldG, oldB, oldA);

    const Uint8 finalAlpha = static_cast<Uint8>(std::lround(std::clamp(stuff.alpha, 0.0f, 1.0f) * 255.0f));

    SDL_SetTextureColorMod(spritemapRenderTexture, finalAlpha, finalAlpha, finalAlpha);
    SDL_SetTextureAlphaMod(spritemapRenderTexture, finalAlpha);
    SDL_RenderTexture(renderer, spritemapRenderTexture, nullptr, nullptr);

    SDL_SetTextureColorMod(spritemapRenderTexture, 255, 255, 255);
    SDL_SetTextureAlphaMod(spritemapRenderTexture, 255);
}

void SDL3Sprite::renderSymbol(const std::string& symbolName, int frame, const AffineMatrix& parentMatrix, const SpritemapStyle& parentStyle, int recursionDepth)
{
    visitSymbolPieces(symbolName, frame, parentMatrix, parentStyle, recursionDepth,
        [&](const std::string& pieceName, const SpritemapPiece&, const AffineMatrix& matrix, const SpritemapStyle& style)
        {
            renderPiece(pieceName, matrix, style);
        }
    );
}


void SDL3Sprite::renderPiece(const std::string& pieceName, const AffineMatrix& matrix, const SpritemapStyle& style)
{
    if (!texture) return;

    auto pieceIt = spritemapData.atlas.find(pieceName);
    if (pieceIt == spritemapData.atlas.end()) return;

    const SpritemapPiece& piece = pieceIt -> second;

    float texWidth, texHeight = 0.0f;
    SDL_GetTextureSize(texture, &texWidth, &texHeight);

    if (texWidth <= 0.0f || texHeight <= 0.0f || piece.w <= 0 || piece.h <= 0) return;

    const float width = static_cast<float>(piece.rotated ? piece.h : piece.w);
    const float height = static_cast<float>(piece.rotated ? piece.w : piece.h);

    auto transformPoint = [&](float x, float y) -> SDL_FPoint {return {matrix.a * x + matrix.c * y + matrix.tx, matrix.b * x + matrix.d * y + matrix.ty};};

    SDL_FPoint positions[] = {
        transformPoint(0.0f, 0.0f),
        transformPoint(width, 0.0f),
        transformPoint(width, height),
        transformPoint(0.0f, height)
    };

    const float left = static_cast<float>(piece.x) / texWidth;
    const float right = static_cast<float>(piece.x + piece.w) / texWidth;
    const float top = static_cast<float>(piece.y) / texHeight;
    const float bottom = static_cast<float>(piece.y + piece.h) / texHeight;

    SDL_FPoint uv[4];

    if (piece.rotated)
    {
        uv[0] = {right, top};
        uv[1] = {right, bottom};
        uv[2] = {left, bottom};
        uv[3] = {left, top};
    }
    else
    {
        uv[0] = {left, top};
        uv[1] = {right, top};
        uv[2] = {right, bottom};
        uv[3] = {left, bottom};
    }

    SDL_Vertex vertices[4];

    for (int i = 0; i < 4; ++i)
    {
        vertices[i].position = positions[i];
        vertices[i].tex_coord = uv[i];

        vertices[i].color = {
            applyColorTransform(stuff.color.r, style.redMultiplier, style.redOffset),
            applyColorTransform(stuff.color.g, style.greenMultiplier, style.greenOffset),
            applyColorTransform(stuff.color.b, style.blueMultiplier, style.blueOffset),
            applyColorTransform(1.0f, style.alphaMultiplier, style.alphaOffset)
        };
    }

    const int indices[6] = {0, 1, 2, 2, 3, 0};

    SDL_RenderGeometry(static_cast<SDL_Renderer*>(globalWindow -> getRenderer()), texture, vertices, 4, indices, 6);
}

SDL3Sprite::~SDL3Sprite()
{
    SDL_DestroyTexture(texture);
    SDL_DestroyTexture(spritemapRenderTexture);
}

SDL3Graphic::SDL3Graphic(const float width, const float height, const Color color)
{
    stuff.width = width;
    stuff.height = height;
    stuff.color = color;
}

void SDL3Graphic::render()
{
    if (!stuff.visible) return;
    BaseGraphic::render();
    if (!renderSelf) return;

    srect.x = 0;
    srect.y = 0;
    srect.w = stuff.width;
    srect.h = stuff.height;

    drect.x = stuff.x * stuff.scaleX;
    drect.y = stuff.y * stuff.scaleY;
    drect.w = srect.w * stuff.scaleX;
    drect.h = srect.h * stuff.scaleY;

    float u0 = srect.x;
    float u1 = srect.x + srect.w;
    float v0 = srect.y;
    float v1 = srect.y + srect.h;

    float rad = stuff.angle * (std::acos(-1.0f) / 180.0f);
    float cosA = cosf(rad);
    float sinA = sinf(rad);

    float centerX = drect.x + (drect.w / 2.0f);
    float centerY = drect.y + (drect.h / 2.0f);

    SDL_FPoint p_tl = {(drect.x + (stuff.skewX / 2.0f)) - centerX, (drect.y + (stuff.skewY / 2.0f)) - centerY};
    SDL_FPoint p_tr = {((drect.x + drect.w) + (stuff.skewX / 2.0f)) - centerX, (drect.y - (stuff.skewY / 2.0f)) - centerY};
    SDL_FPoint p_br = {((drect.x + drect.w) - (stuff.skewX / 2.0f)) - centerX, ((drect.y + drect.h) - (stuff.skewY / 2.0f)) - centerY};
    SDL_FPoint p_bl = {(drect.x - (stuff.skewX / 2.0f)) - centerX, ((drect.y + drect.h) + (stuff.skewY / 2.0f)) - centerY};

    SDL_Vertex vertices[4] = {
        {{centerX + (p_tl.x * cosA - p_tl.y * sinA), centerY + (p_tl.x * sinA + p_tl.y * cosA)}, {stuff.color.r, stuff.color.g, stuff.color.b, stuff.alpha}, {u0, v0}},
        {{centerX + (p_tr.x * cosA - p_tr.y * sinA), centerY + (p_tr.x * sinA + p_tr.y * cosA)}, {stuff.color.r, stuff.color.g, stuff.color.b, stuff.alpha}, {u1, v0}},
        {{centerX + (p_br.x * cosA - p_br.y * sinA), centerY + (p_br.x * sinA + p_br.y * cosA)}, {stuff.color.r, stuff.color.g, stuff.color.b, stuff.alpha}, {u1, v1}},
        {{centerX + (p_bl.x * cosA - p_bl.y * sinA), centerY + (p_bl.x * sinA + p_bl.y * cosA)}, {stuff.color.r, stuff.color.g, stuff.color.b, stuff.alpha}, {u0, v1}}
    };

    int daindices[6] = {0, 1, 2, 2, 3, 0};

    SDL_RenderGeometry(static_cast<SDL_Renderer*>(globalWindow -> getRenderer()), nullptr, vertices, 4, daindices, 6);
}

SDL3Graphic::~SDL3Graphic()
{
    
}