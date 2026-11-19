#pragma once

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>

#include <functional>
#include <string>

#include "../basesprite.hpp"

class SDL3Sprite : public BaseSprite 
{
    public:
        SDL3Sprite(const std::string& path, const std::string& type = "none");
        ~SDL3Sprite() override;

        void create(const std::string& path, const std::string& type) override;
        void render() override;

        void renderSpritemap(int frame);
        void renderSymbol(const std::string& symbolName, int frame, const AffineMatrix& parentMatrix, const SpritemapStyle& parentStyle, int recursionDepth);
        void renderPiece(const std::string& pieceName, const AffineMatrix& matrix, const SpritemapStyle& style);
    private:
        SDL_Texture *texture = nullptr;

        SDL_Texture* spritemapRenderTexture = nullptr;
        int spritemapRenderWidth = 0;
        int spritemapRenderHeight = 0;

        void visitSymbolPieces(const std::string& symbolName, int frame, const AffineMatrix& parentMatrix, const SpritemapStyle& parentStyle, int recursionDepth,
            const std::function<void(const std::string&, const SpritemapPiece&, const AffineMatrix&, const SpritemapStyle&)>& callback) const;
};

class SDL3Graphic : public BaseGraphic
{
    public:
        SDL3Graphic(const float width = 0.0f, const float height = 0.0f, const Color color = {});
        ~SDL3Graphic() override;

        void render() override;
};