#include "atlastext.hpp"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <iostream>

std::map<AtlasFont, AtlasFontData> AtlasText::fonts;

AtlasFontData::AtlasFontData(const std::string& path, const std::string& type)
{
    atlasPath = path;
    spriteType = type;

    FunkinSprite temp(path, type);

    frameNames = temp.getAtlasFrameNames();
    maxHeight = temp.getAtlasMaxHeight();

    bool containsUpper = false;
    bool containsLower = false;

    for (const std::string& name : frameNames)
    {
        if (name.empty()) continue;

        if (name.size() >= 2 && std::all_of(name.begin() + 1, name.end(), [](unsigned char c) { return std::isdigit(c); }))
        {
            char c = name[0];

            if (c >= 'A' && c <= 'Z') containsUpper = true;
            if (c >= 'a' && c <= 'z') containsLower = true;
        }
    }

    if (containsUpper && !containsLower) caseAllowed = Case::Upper;
    else if (containsLower && !containsUpper) caseAllowed = Case::Lower;
    else caseAllowed = Case::Both;
}

AtlasText::AtlasText(const std::string& text, AtlasFont fontName, const std::string& atlasPath) : FunkinSprite(atlasPath, "sparrow"), x(0), y(0)
{
    auto it = fonts.find(fontName);

    if (it == fonts.end())
    {
        it = fonts.emplace(std::piecewise_construct, std::forward_as_tuple(fontName), std::forward_as_tuple(atlasPath, "sparrow")).first;
    }

    font = &it -> second;

    setRenderSelf(false);
    setText(text);
}

std::string AtlasText::restrictCase(const std::string& str) const
{
    std::string result = str;
    if (font -> caseAllowed == Case::Both) return result;

    for (char& c : result)
    {
        unsigned char uc = static_cast<unsigned char>(c);

        if (font -> caseAllowed == Case::Upper) c = static_cast<char>(std::toupper(uc));
        else if (font -> caseAllowed == Case::Lower) c = static_cast<char>(std::tolower(uc));
    }

    return result;
}

void AtlasText::setText(const std::string& value)
{
    std::string caseValue = restrictCase(value);
    std::string caseText = restrictCase(currentText);

    currentText = value;

    if (caseText == caseValue) return;

    if (caseValue.size() >= caseText.size() && caseValue.compare(0, caseText.size(), caseText) == 0)
    {
        appendTextCased(caseValue.substr(caseText.size()));
        return;
    }

    for (FunkinSprite* character : characters)
    {
        character -> getParams().visible = false;
    }

    appendTextCased(caseValue);
}

void AtlasText::appendText(const std::string& str)
{
    if (str.empty()) return;

    currentText += str;
    appendTextCased(restrictCase(str));
}

std::string AtlasText::getText() const
{
    return currentText;
}

void AtlasText::appendTextCased(const std::string& str)
{
    float xPos = 0.0f;
    float yPos = 0.0f;

    FunkinSprite* lastChar = nullptr;

    for (auto it = characters.rbegin(); it != characters.rend(); ++it)
    {
        if ((*it) -> getParams().visible)
        {
            lastChar = *it;
            break;
        }
    }

    if (lastChar)
    {
        const SpriteRenderParams& params = lastChar -> getParams();

        xPos = params.x - x + lastChar -> getWidth();
        yPos = params.y - y + lastChar -> getHeight() - getMaxHeight();
    }

    for (size_t i = 0; i < str.size();)
    {
        unsigned char c = static_cast<unsigned char>(str[i]);

        size_t charLength = 1;

        if ((c & 0xE0) == 0xC0) charLength = 2;
        else if ((c & 0xF0) == 0xE0) charLength = 3;
        else if ((c & 0xF8) == 0xF0) charLength = 4;

        std::string character = str.substr(i, charLength);
        i += charLength;

        if (character == " ")
        {
            xPos += 40.0f;
            continue;
        }

        if (character == "\n")
        {
            xPos = 0.0f;
            yPos += getMaxHeight();
            continue;
        }

        std::string animationPrefix = character;

        if (character == "&") animationPrefix = "-andpersand-";
        else if (character == "😠") animationPrefix = "-angry faic-";
        else if (character == "'") animationPrefix = "-apostraphie-";
        else if (character == "\\") animationPrefix = "-back slash-";
        else if (character == ",") animationPrefix = "-comma-";
        else if (character == "-") animationPrefix = "-dash-";
        else if (character == "↓") animationPrefix = "-down arrow-";
        else if (character == "”") animationPrefix = "-end quote-";
        else if (character == "!") animationPrefix = "-exclamation point-";
        else if (character == "/") animationPrefix = "-forward slash-";
        else if (character == ">") animationPrefix = "-greater than-";
        else if (character == "♥" || character == "♡") animationPrefix = "-heart-";
        else if (character == "←") animationPrefix = "-left arrow-";
        else if (character == "<") animationPrefix = "-less than-";
        else if (character == "*") animationPrefix = "-multiply x-";
        else if (character == ".") animationPrefix = "-period-";
        else if (character == "?") animationPrefix = "-question mark-";
        else if (character == "→") animationPrefix = "-right arrow-";
        else if (character == "“") animationPrefix = "-start quote-";
        else if (character == "↑") animationPrefix = "-up arrow-";

        FunkinSprite* charSprite = nullptr;

        for (FunkinSprite* existing : characters)
        {
            if (!existing -> getParams().visible)
            {
                charSprite = existing;
                break;
            }
        }

        if (charSprite)
        {
            charSprite -> getParams().visible = true;
            charSprite -> getParams().alpha = 1.0f;
            charSprite -> addAnim("exist", animationPrefix, 24, true);
            charSprite -> playAnim("exist");
        }
        else
        {
            auto newCharacter = std::make_unique<FunkinSprite>(font -> atlasPath, font -> spriteType);

            newCharacter -> addAnim("exist", animationPrefix, 24, true);
            newCharacter -> playAnim("exist");
            
            charSprite = newCharacter.get();
            characters.push_back(charSprite);

            addChild(std::move(newCharacter));
        }

        charSprite -> getParams().x = x + xPos;
        charSprite -> getParams().y = y + yPos + getMaxHeight() - charSprite -> getHeight();

        xPos += charSprite -> getWidth();
    }
}

float AtlasText::getWidth() const
{
    float width = 0.0f;
    size_t charIndex = 0;

    for (size_t i = 0; i < currentText.size();)
    {
        unsigned char c = static_cast<unsigned char>(currentText[i]);

        size_t length = 1;
        if ((c & 0xE0) == 0xC0) length = 2; else if ((c & 0xF0) == 0xE0) length = 3; else if ((c & 0xF8) == 0xF0) length = 4;

        std::string character = currentText.substr(i, length);
        i += length;

        if (character == " ")
        {
            width += 40.0f;
        }
        else if (character != "\n")
        {
            if (charIndex < characters.size()) width += characters[charIndex] -> getWidth();
            charIndex++;
        }
    }

    return width;
}

float AtlasText::getHeight() const
{
    if (currentText.empty()) return 0.0f;
    int lines = 1;

    for (char c : currentText)
    {
        if (c == '\n') lines++;
    }

    return getMaxHeight() * lines;
}

float AtlasText::getMaxHeight() const
{
    return font ? font -> maxHeight : 0.0f;
}

void AtlasText::setPosition(float x, float y)
{
    float dx = x - this -> x;
    float dy = y - this -> y;

    this -> x = x;
    this -> y = y;

    for (FunkinSprite* character : characters)
    {
        if (!character -> getParams().visible) continue;

        character -> getParams().x += dx;
        character -> getParams().y += dy;
    }
}

void AtlasText::setAlpha(float alpha)
{
    for (FunkinSprite* character : characters)
    {
        if (character -> getParams().visible) character -> getParams().alpha = alpha;
    }
}
