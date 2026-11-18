#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>

#include "../import.hpp"

enum class AtlasFont
{
    DEFAULT,
    BOLD,
    FREEPLAY_CLEAR
};

inline std::string AtlasFontToString(AtlasFont font)
{
    switch (font)
    {
        case AtlasFont::DEFAULT: return "default";
        case AtlasFont::BOLD: return "bold";
        case AtlasFont::FREEPLAY_CLEAR: return "freeplay-clear";
        default: return "default";
    }
}

enum class Case
{
    Both,
    Upper,
    Lower
};

class AtlasFontData
{
    public:
        std::string atlasPath;
        std::string spriteType;

        float maxHeight = 0.0f;
        Case caseAllowed = Case::Both;

        std::vector<std::string> frameNames;

        AtlasFontData(const std::string& path, const std::string& type);
};

class AtlasText : public FunkinSprite
{
    private:
        static std::map<AtlasFont, AtlasFontData> fonts;

        std::string currentText;
        float x;
        float y;

        AtlasFontData* font = nullptr;

        std::string restrictCase(const std::string& str) const;
        void appendTextCased(const std::string& text);

        std::vector<FunkinSprite*> characters;

    public:
        AtlasText(const std::string& text, AtlasFont fontName, const std::string& atlasPath);

        void setText(const std::string& text);
        void appendText(const std::string& text);

        std::string getText() const;

        float getWidth() const;
        float getHeight() const;
        float getMaxHeight() const;

        void setPosition(float x, float y);
        void setAlpha(float alpha);
};