#include <algorithm>
#include <fstream>
#include <sstream>
#include <cmath>
#include <cctype>
#include <cstring>
#include <string>
#include <cstdlib>
#include <functional>
#include <limits>
#include <iostream>
#include <vector>
#include <cctype>

#include <pugixml.hpp>

#include "basesprite.hpp"

#if defined(PLATFORM_WINDOWS) || defined(PLATFORM_LINUX)
#include "sdl3/sdl3sprite.hpp"
#endif

// Spritemap parser stuff that I totally didnt steal from Snake Engine

static void skipJsonWhitespace(const std::string& src, size_t& p)
{
    while (p < src.size() &&
           std::isspace(static_cast<unsigned char>(src[p])))
    {
        ++p;
    }
}

static size_t findJsonKey(const std::string& src, const std::string& key, size_t startPos = 0)
{
    size_t p = startPos;

    while (p < src.size())
    {
        char c = src[p];

        if (c == '"')
        {
            size_t stringStart = p++;

            while (p < src.size())
            {
                if (src[p] == '\\')
                {
                    p += (p + 1 < src.size()) ? 2 : 1;
                    continue;
                }

                if (src[p++] == '"') break;
            }

            const std::string token = src.substr(stringStart + 1, p - stringStart - 2);

            size_t after = p;
            skipJsonWhitespace(src, after);

            if (token == key && after < src.size() && src[after] == ':') return after + 1;
            continue;
        }

        ++p;
    }

    return std::string::npos;
}

static std::string jsonReadValue(const std::string& src, size_t p)
{
    skipJsonWhitespace(src, p);
    if (p >= src.size()) return "";

    const size_t start = p;
    const char first = src[p];

    if (first == '"' || first == '{' || first == '[')
    {
        const char closing = first == '"' ? '"' : first == '{' ? '}' : ']';
        int depth = 0;
        bool inString = false;

        for (; p < src.size(); ++p)
        {
            char c = src[p];

            if (inString)
            {
                if (c == '\\')
                {
                    if (p + 1 < src.size()) ++p;
                }
                else if (c == '"')
                {
                    inString = false;

                    if (first == '"') return src.substr(start, p - start + 1);
                }

                continue;
            }

            if (c == '"')
            {
                inString = true;
                continue;
            }

            if (first == '{' || first == '[')
            {
                if (c == first) ++depth;
                else if (c == closing)
                {
                    --depth;
                    if (depth == 0) return src.substr(start, p - start + 1);
                }
            }
        }

        return "";
    }

    while (p < src.size() && src[p] != ',' && src[p] != '}' && src[p] != ']' && !std::isspace(static_cast<unsigned char>(src[p]))) ++p;
    return src.substr(start, p - start);
}

static std::string jsonGetRaw(const std::string& src, const std::string& key, size_t startPos = 0)
{
    size_t p = findJsonKey(src, key, startPos);
    if (p == std::string::npos) return "";

    std::string value = jsonReadValue(src, p);
    if (value.size() >= 2 && value.front() == '"' && value.back() == '"') value = value.substr(1, value.size() - 2);
    
    return value;
}

static float fast_atof(const char* p, char** end = nullptr)
{
    if (!p)
    {
        if (end) *end = nullptr;
        return 0.0f;
    }

    while (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n') ++p;

    char* parsedEnd = nullptr;
    float value = std::strtof(p, &parsedEnd);

    if (end) *end = parsedEnd;
    if (parsedEnd == p) return 0.0f;

    return value;
}

static float jsonGetFloat(const std::string& src, const std::string& key, float def = 0.0f)
{
    std::string value = jsonGetRaw(src, key);
    if (value.empty() || value == "null") return def;

    char* end = nullptr;
    float result = fast_atof(value.c_str(), &end);
    if (end == value.c_str() || *end != '\0') return def;

    return result;
}

static int jsonGetInt(const std::string& src, const std::string& key, int def = 0)
{
    std::string value = jsonGetRaw(src, key);
    if (value.empty() || value == "null") return def;

    char* end = nullptr;
    float result = fast_atof(value.c_str(), &end);
    if (end == value.c_str() || *end != '\0') return def;

    return static_cast<int>(result);
}

static std::string jsonGetObject(const std::string& src, const std::string& key, size_t startPos = 0)
{
    size_t p = findJsonKey(src, key, startPos);
    if (p == std::string::npos) return "";

    skipJsonWhitespace(src, p);
    if (p >= src.size() || src[p] != '{') return "";

    std::string result = jsonReadValue(src, p);
    return (!result.empty() && result.front() == '{') ? result : "";
}

static std::string jsonGetArray(const std::string& src, const std::string& key, size_t startPos = 0)
{
    size_t p = findJsonKey(src, key, startPos);
    if (p == std::string::npos) return "";

    skipJsonWhitespace(src, p);
    if (p >= src.size() || src[p] != '[') return "";

    std::string result = jsonReadValue(src, p);
    return (!result.empty() && result.front() == '[') ? result : "";
}

static AffineMatrix parseMX(const std::string& src, size_t startPos = 0)
{
    AffineMatrix matrix;
    std::string raw = jsonGetArray(src, "MX", startPos);
    if (raw.empty()) return matrix;

    size_t p = 1;
    float values[6] = {1, 0, 0, 1, 0, 0};

    for (int i = 0; i < 6; ++i)
    {
        skipJsonWhitespace(raw, p);

        if (p < raw.size() && raw[p] == ',')
        {
            ++p;
            skipJsonWhitespace(raw, p);
        }

        if (p >= raw.size() || raw[p] == ']') break;

        char* end = nullptr;
        values[i] = fast_atof(raw.c_str() + p, &end);

        if (end == raw.c_str() + p) return AffineMatrix{};
        p = static_cast<size_t>(end - raw.c_str());
    }

    matrix.a  = values[0];
    matrix.b  = values[1];
    matrix.c  = values[2];
    matrix.d  = values[3];
    matrix.tx = values[4];
    matrix.ty = values[5];

    return matrix;
}

static Pivot parseTRP(const std::string& src, size_t startPos = 0)
{
    Pivot pivot;

    std::string raw = jsonGetObject(src, "TRP", startPos);
    if (raw.empty()) return pivot;

    pivot.x = jsonGetFloat(raw, "x", 0.0f);
    pivot.y = jsonGetFloat(raw, "y", 0.0f);

    return pivot;
}

static void forEachObject(const std::string& arr, std::function<void(const std::string&)> callback)
{
    size_t p = 0;

    while (p < arr.size())
    {
        if (arr[p] != '{')
        {
            ++p;
            continue;
        }

        std::string object = jsonReadValue(arr, p);
        if (object.empty()) return;

        callback(object);
        p += object.size();
    }
}

int SpritemapSymbol::duration() const
{
    int maxDuration = 0;

    for (const auto& layer : layers)
    {
        for (const auto& keyframe : layer.keyframes)
        {
            const int end = keyframe.index + keyframe.duration;
            if (end > maxDuration) maxDuration = end;
        }
    }

    return maxDuration;
}

const SpritemapKeyframe* SpritemapSymbol::getKeyframe(int layerIdx, int frame) const
{
    if (layerIdx < 0 || layerIdx >= (int)layers.size()) return nullptr;
    const auto& layer = layers[layerIdx];
    const SpritemapKeyframe* active = nullptr;
    for (const auto& kf : layer.keyframes)
    {
        if (frame >= kf.index && frame < kf.index + kf.duration)
        {
            active = &kf;
        }
    }
    return active;
}

bool SpritemapParser::parseAtlas(const std::string& path, SpritemapData& out)
{
    std::ifstream f(path);
    if (!f.is_open()) return false;
    std::ostringstream ss;
    ss << f.rdbuf();
    std::string src = ss.str();

    std::string atlasObj = jsonGetObject(src, "ATLAS");
    if (atlasObj.empty()) return false;
    std::string spritesArr = jsonGetArray(atlasObj, "SPRITES");
    if (spritesArr.empty()) return false;

    forEachObject(spritesArr, [&](const std::string& item)
    {
        std::string sprObj = jsonGetObject(item, "SPRITE");
        if (sprObj.empty()) return;
        SpritemapPiece piece;
        piece.name = jsonGetRaw(sprObj, "name");
        piece.x = jsonGetInt(sprObj, "x");
        piece.y = jsonGetInt(sprObj, "y");
        piece.w = jsonGetInt(sprObj, "w");
        piece.h = jsonGetInt(sprObj, "h");
        std::string rot = jsonGetRaw(sprObj, "rotated");
        piece.rotated = (rot == "true" || rot == "1");
        out.atlas[piece.name] = piece;
    });

    return !out.atlas.empty();
}

static SpritemapStyle parseSpritemapStyle(const std::string& siStr)
{
    SpritemapStyle style;

    std::string colorObj = jsonGetObject(siStr, "C");

    if (!colorObj.empty())
    {
        style.hasColor = true;

        std::string mode = jsonGetRaw(colorObj, "M");

        auto getColorValue = [&](const std::string& key, float fallback)
        {
            std::string raw = jsonGetRaw(colorObj, key);
            if (raw.empty())
                return fallback;

            try
            {
                return std::stof(raw);
            }
            catch (...)
            {
                return fallback;
            }
        };

        if (mode == "CA")
        {
            style.alphaMultiplier = getColorValue("AM", 1.0f);
        }
        else if (mode == "AD")
        {
            style.redMultiplier   = getColorValue("RM", 1.0f);
            style.greenMultiplier = getColorValue("GM", 1.0f);
            style.blueMultiplier  = getColorValue("BM", 1.0f);
            style.alphaMultiplier = getColorValue("AM", 1.0f);

            style.redOffset   = getColorValue("RO", 0.0f);
            style.greenOffset = getColorValue("GO", 0.0f);
            style.blueOffset  = getColorValue("BO", 0.0f);
            style.alphaOffset = getColorValue("AO", 0.0f);
        }
    }

    std::string blend = jsonGetRaw(siStr, "B");

    if (!blend.empty())
    {
        style.blend = blend;
        style.hasBlend = true;
    }

    return style;
}

static SpritemapSymbol parseSymbol(const std::string& symStr)
{
    SpritemapSymbol sym;
    sym.name = jsonGetRaw(symStr, "SN");

    std::string tlObj = jsonGetObject(symStr, "TL");
    if (tlObj.empty()) return sym;
    std::string layersArr = jsonGetArray(tlObj, "L");
    if (layersArr.empty()) return sym;

    forEachObject(layersArr, [&](const std::string& layerStr)
    {
        SpritemapLayer layer;
        layer.name = jsonGetRaw(layerStr, "LN");

        std::string frArr = jsonGetArray(layerStr, "FR");
        if (frArr.empty()) return;

        forEachObject(frArr, [&](const std::string& frStr)
        {
            SpritemapKeyframe kf;
            kf.index    = jsonGetInt(frStr, "I");
            kf.duration = jsonGetInt(frStr, "DU");

            std::string elemArr = jsonGetArray(frStr, "E");
            if (!elemArr.empty())
            {
                forEachObject(elemArr, [&](const std::string& elemStr)
                {
                    SpritemapElement elem;

                    std::string asiObj = jsonGetObject(elemStr, "ASI");
                    if (!asiObj.empty())
                    {
                        elem.isAtlasSprite = true;
                        elem.asi.pieceName = jsonGetRaw(asiObj, "N");
                        elem.asi.mx = parseMX(asiObj);
                        kf.elements.push_back(elem);
                        return;
                    }

                    std::string siObj = jsonGetObject(elemStr, "SI");
                    if (!siObj.empty())
                    {
                        elem.isAtlasSprite = false;
                        elem.si.symbolName = jsonGetRaw(siObj, "SN");
                        elem.si.loopType = jsonGetRaw(siObj, "LP");
                        elem.si.firstFrame = jsonGetInt(siObj, "FF");
                        elem.si.mx = parseMX(siObj);
                        elem.si.trp = parseTRP(siObj);

                        elem.si.style = parseSpritemapStyle(siObj);

                        kf.elements.push_back(elem);
                    }
                });
            }

            layer.keyframes.push_back(kf);
        });

        sym.layers.push_back(layer);
    });

    return sym;
}

bool SpritemapParser::parseAnimation(const std::string& path, SpritemapData& out)
{
    std::ifstream f(path);
    if (!f.is_open()) return false;
    std::ostringstream ss;
    ss << f.rdbuf();
    std::string src = ss.str();

    std::string mdObj = jsonGetObject(src, "MD");
    if (!mdObj.empty())
    {
        out.canvasW = jsonGetFloat(mdObj, "W", 1280.f);
        out.canvasH = jsonGetFloat(mdObj, "H", 720.f);
        out.frameRate = jsonGetInt(mdObj, "FRT", 24);
    }

    std::string anObj = jsonGetObject(src, "AN");
    out.rootSymbolName = jsonGetRaw(anObj, "SN");
    if (!anObj.empty() && !out.rootSymbolName.empty())
    {
        SpritemapSymbol rootSym = parseSymbol(anObj);
        out.symbols[out.rootSymbolName] = std::move(rootSym);
    }

    if (!anObj.empty())
    {
        std::string tlObj = jsonGetObject(anObj, "TL");
        if (!tlObj.empty())
        {
            std::string layersArr = jsonGetArray(tlObj, "L");
            forEachObject(layersArr, [&](const std::string& layerStr)
            {
                std::string frArr = jsonGetArray(layerStr, "FR");
                if (frArr.empty()) return;

                forEachObject(frArr, [&](const std::string& frStr)
                {
                    std::string animName = jsonGetRaw(frStr, "N");
                    if (!animName.empty())
                    {
                        SpritemapAnimation anim;
                        anim.name = animName;
                        anim.startFrame = jsonGetInt(frStr, "I");
                        anim.duration = jsonGetInt(frStr, "DU");
                        anim.symbolName = out.rootSymbolName;

                        std::string elemArr = jsonGetArray(frStr, "E");
                        if (!elemArr.empty())
                        {
                            std::string firstElem;
                            forEachObject(elemArr, [&](const std::string& elem)
                            {
                                if (firstElem.empty()) firstElem = elem;
                            });
                            std::string siObj = jsonGetObject(firstElem, "SI");
                            if (!siObj.empty())
                            {
                                anim.loopType = jsonGetRaw(siObj, "LP");
                            }
                        }

                        out.animations.push_back(anim);
                    }
                });
            });
        }
    }

    std::string sdObj = jsonGetObject(src, "SD");
    if (sdObj.empty()) return false;
    std::string symbolsArr = jsonGetArray(sdObj, "S");
    if (symbolsArr.empty()) return false;

    forEachObject(symbolsArr, [&](const std::string& symStr)
    {
        SpritemapSymbol sym = parseSymbol(symStr);
        if (!sym.name.empty())
        {
            out.symbols[sym.name] = std::move(sym);
        }
    });

    return true;
}

bool SpritemapParser::parse(const std::string& atlasJsonPath, const std::string& animJsonPath, SpritemapData& out)
{
    out.atlas.clear();
    out.symbols.clear();
    out.animations.clear();

    if(!parseAtlas(atlasJsonPath, out)) return false;
    if(!parseAnimation(animJsonPath, out)) return false;

    return true;
}

bool BaseSprite::loadSpritemap(const std::string& path)
{
    std::string basePath = path;

    while (!basePath.empty() && (basePath.back() == '/' || basePath.back() == '\\'))
    {
        basePath.pop_back();
    }

    const std::string filename = "spritemap1.json";

    std::string atlasPath = basePath + "/spritemap1.json";
    std::string animationPath = basePath + "/Animation.json";

    SpritemapData parsed;

    if (!SpritemapParser::parse(atlasPath, animationPath, parsed))
    {
        std::cerr << "Failed to load Animate spritemap:\n" << "  Atlas: " << atlasPath << '\n' << "  Animation: " << animationPath << '\n';
        return false;
    }

    spritemapData = std::move(parsed);
    spriteType = "animateatlas";

    stuff.width = spritemapData.canvasW;
    stuff.height = spritemapData.canvasH;

    drect.w = spritemapData.canvasW;
    drect.h = spritemapData.canvasH;
    
    return true;
}

std::string BaseSprite::getAnimName(const std::string& frameName, int& frameIndex)
{
    size_t pos = frameName.size();

    while (pos > 0 && std::isdigit(static_cast<unsigned char>(frameName[pos - 1]))) pos -= 1;
    if (pos == frameName.size())
    {
        frameIndex = 0;
        return frameName;
    }

    frameIndex = std::stoi(frameName.substr(pos));

    return frameName.substr(0, pos);
}

void BaseSprite::loadAtlas(const std::string& path)
{
    pugi::xml_document doc;
    doc.load_file(path.c_str());

    pugi::xml_node atlas = doc.child("TextureAtlas");

    for (pugi::xml_node node : atlas.children("SubTexture"))
    {
        AtlasFrame frame;

        frame.name = node.attribute("name").as_string();
        frame.x = node.attribute("x").as_float();
        frame.y = node.attribute("y").as_float();
        frame.width = node.attribute("width").as_float();
        frame.height = node.attribute("height").as_float();
        frame.frameX = node.attribute("frameX") ? node.attribute("frameX").as_float() : 0.0f;
        frame.frameY = node.attribute("frameY") ? node.attribute("frameY").as_float() : 0.0f;
        frame.frameWidth = node.attribute("frameWidth") ? node.attribute("frameWidth").as_float() : frame.width;
        frame.frameHeight = node.attribute("frameHeight") ? node.attribute("frameHeight").as_float() : frame.height;

        std::string animName = getAnimName(frame.name, frame.index);

        auto it = std::find_if(anims.begin(), anims.end(), [&](const Animation& anim) {return anim.name == animName;});

        if (it == anims.end())
        {
            Animation anim;

            anim.name = animName;
            anim.frames.push_back(frame);

            anims.push_back(anim);
        }
        else
        {
            it -> frames.push_back(frame);
        }
    }

    for (Animation& anim : anims)
    {
        std::sort(anim.frames.begin(), anim.frames.end(), [](const AtlasFrame& a, const AtlasFrame& b) {return a.index < b.index;});
    }

    if (!anims.empty() && !anims[0].frames.empty())
    {
        curAnim = 0;
        curFrame = 0;
    }
}

void BaseSprite::addAnimFrames(const std::string& name, const std::string& prefix, const std::vector<int>& indices, float fps, bool looped, bool flipX, bool flipY)
{
    auto it = std::find_if(anims.begin(), anims.end(), [&](const Animation& anim) {return anim.name == prefix;});

    if (it == anims.end())
        return;

    Animation anim;

    anim.name = name;
    anim.frames.clear();

    for (int index : indices)
    {
        auto frameIt = std::find_if(it -> frames.begin(), it -> frames.end(), [&](const AtlasFrame& frame) {return frame.index == index;});
        if (frameIt != it -> frames.end()) anim.frames.push_back(*frameIt);
    }

    anim.fps = fps;
    anim.looped = looped;
    anim.flipX = flipX;
    anim.flipY = flipY;

    anims.push_back(anim);
}

void BaseSprite::addAnim(const std::string& name, const std::string& prefix, float fps, bool looped, bool flipX, bool flipY)
{
    auto it = std::find_if(anims.begin(), anims.end(), [&](const Animation& anim) {return anim.name == prefix;});

    if (it == anims.end()) return;

    Animation anim = *it;

    anim.name = name;
    anim.fps = fps;
    anim.looped = looped;
    anim.flipX = flipX;
    anim.flipY = flipY;

    auto existing = std::find_if(anims.begin(), anims.end(), [&](const Animation& current) {return current.name == name;});
    if (existing != anims.end()) *existing = std::move(anim); else anims.push_back(std::move(anim));
}

void BaseSprite::addAnim(const std::string& name, const std::string& prefix, const std::vector<int>& indices, float fps, bool looped, bool flipX, bool flipY)
{
    addAnimFrames(name, prefix, indices, fps, looped, flipX, flipY);
}

void BaseSprite::addSpritemapAnim(const std::string& name, const std::string& symbolName, const std::vector<int>& indices, float fps, bool loop)
{
    auto sourceIt = std::find_if(spritemapData.animations.begin(), spritemapData.animations.end(), [&](const SpritemapAnimation& anim) {return anim.name == symbolName;});

    Animation anim;
    anim.name = name;
    anim.fps = fps > 0.0f ? fps : static_cast<float>(spritemapData.frameRate > 0 ? spritemapData.frameRate : 24);
    anim.looped = loop;

    int startFrame = 0;
    int duration = 0;

    if (sourceIt != spritemapData.animations.end())
    {
        startFrame = sourceIt -> startFrame;
        duration = sourceIt -> duration;
    }
    else
    {
        auto symbolIt = spritemapData.symbols.find(symbolName);

        if (symbolIt == spritemapData.symbols.end())
        {
            std::cerr << "BaseSprite: cannot add spritemap animation '" << name << "': symbol or animation '" << symbolName << "' was not found.\n";
            return;
        }

        duration = symbolIt -> second.duration();
    }

    if (indices.empty())
    {
        for (int i = 0; i < duration; ++i)
        {
            AtlasFrame frame;
            frame.index = startFrame + i;
            anim.frames.push_back(frame);
        }
    }
    else
    {
        for (int index : indices)
        {
            if (index < 0) continue;

            AtlasFrame frame;
            frame.index = startFrame + index;
            anim.frames.push_back(frame);
        }
    }

    if (anim.frames.empty())
    {
        std::cerr << "BaseSprite: animation '" << name << "' has no frames.\n";
        return;
    }

    auto existing = std::find_if(anims.begin(), anims.end(), [&](const Animation& item) {return item.name == name;});
    if (existing != anims.end()) *existing = std::move(anim); else anims.push_back(std::move(anim));
}

void BaseSprite::playAnim(const std::string& name)
{
    for (int i = 0; i < static_cast<int>(anims.size()); i++)
    {
        if (anims[i].name == name)
        {
            curAnim = i;
            curFrame = 0;

            animTimer = 0.0f;
            animFPS = anims[i].fps;
            animLoop = anims[i].looped;
            animPlaying = true;

            return;
        }
    }
}

std::vector<std::string> BaseSprite::getAtlasFrameNames() const
{
    std::vector<std::string> names;

    for (const Animation& anim : anims)
    {
        for (const AtlasFrame& frame : anim.frames)
        {
            names.push_back(frame.name);
        }
    }

    return names;
}

void BaseSprite::update(float elapsed)
{
    if (animPlaying && curAnim >= 0 && curAnim < static_cast<int>(anims.size()))
    {
        Animation& anim = anims[curAnim];

        if (!anim.frames.empty())
        {
            animTimer += elapsed;
            float frameTime = 1.0f / animFPS;

            while (animTimer >= frameTime)
            {
                animTimer -= frameTime;
                curFrame++;

                if (curFrame >= static_cast<int>(anim.frames.size()))
                {
                    if (animLoop)
                    {
                        curFrame = 0;
                    }
                    else
                    {
                        curFrame = static_cast<int>(anim.frames.size()) - 1;
                        animPlaying = false;
                        break;
                    }
                }
            }
        }
    }

    for (auto& child : children) child -> update(elapsed);
}

BaseSprite::~BaseSprite() 
{
    
}

void BaseSprite::render() 
{
    for (auto& child : children) child -> render();
}

float BaseSprite::getWidth()
{
    if (spriteType == "sparrow" && curAnim >= 0 && curAnim < static_cast<int>(anims.size()))
    {
        const Animation& anim = anims[curAnim];

        if (curFrame >= 0 && curFrame < static_cast<int>(anim.frames.size()))
        {
            return anim.frames[curFrame].width * stuff.scaleX;
        }
    }

    return drect.w;
}

float BaseSprite::getHeight()
{
    if (spriteType == "sparrow" && curAnim >= 0 && curAnim < static_cast<int>(anims.size()))
    {
        const Animation& anim = anims[curAnim];

        if (curFrame >= 0 && curFrame < static_cast<int>(anim.frames.size()))
        {
            return anim.frames[curFrame].height * stuff.scaleY;
        }
    }

    return drect.h;
}

float BaseSprite::getAtlasMaxHeight() const
{
    float maxHeight = 0.0f;

    for (const Animation& anim : anims)
    {
        for (const AtlasFrame& frame : anim.frames)
        {
            maxHeight = std::max(maxHeight, frame.frameHeight > 0.0f ? frame.frameHeight : frame.height);
        }
    }

    return maxHeight;
}

void BaseSprite::addChild(std::unique_ptr<FunkinSprite> child)
{
    if (child) children.push_back(std::move(child));
}

void BaseSprite::clearChildren()
{
    children.clear();
}

void BaseGraphic::update(float elapsed)
{
    for (auto& child : children) child -> update(elapsed);
}

BaseGraphic::~BaseGraphic() 
{
    
}

void BaseGraphic::render() 
{
    for (auto& child : children) child -> render();
}

float BaseGraphic::getWidth()
{
    return drect.w;
}

float BaseGraphic::getHeight()
{
    return drect.h;
}

void BaseGraphic::addChild(std::unique_ptr<FunkinGraphic> child)
{
    if (child) children.push_back(std::move(child));
}

void BaseGraphic::clearChildren()
{
    children.clear();
}

FunkinSprite::FunkinSprite(const std::string& path, const std::string& type)
{
    #if defined(PLATFORM_WINDOWS) || defined(PLATFORM_LINUX)
    sprite = std::make_unique<SDL3Sprite>(path, type);
    #endif
}

void FunkinSprite::update(float elapsed)
{
    sprite -> update(elapsed);
}

void FunkinSprite::render()
{
    sprite -> render();
}

void FunkinSprite::addAnim(const std::string& name, const std::string& prefix, float fps, bool looped, bool flipX, bool flipY)
{
    sprite -> addAnim(name, prefix, fps, looped, flipX, flipY);
}

void FunkinSprite::addAnim(const std::string& name, const std::string& prefix, const std::vector<int>& indices, float fps, bool looped, bool flipX, bool flipY)
{
    sprite -> addAnim(name, prefix, indices, fps, looped, flipX, flipY);
}

void FunkinSprite::addSpritemapAnim(const std::string& name, const std::string& symbolName, const std::vector<int>& indices, float fps, bool loop)
{
    sprite -> addSpritemapAnim(name, symbolName, indices, fps, loop);
}

void FunkinSprite::playAnim(const std::string& name)
{
    sprite -> playAnim(name);
}

std::vector<std::string> FunkinSprite::getAtlasFrameNames() const
{
    return sprite -> getAtlasFrameNames();
}

SpriteRenderParams& FunkinSprite::getParams()
{
    return sprite -> stuff;
}

float FunkinSprite::getWidth()
{
    return sprite -> getWidth();
}

float FunkinSprite::getHeight()
{
    return sprite -> getHeight();
}

float FunkinSprite::getAtlasMaxHeight() const
{
    return sprite -> getAtlasMaxHeight();
}

void FunkinSprite::addChild(std::unique_ptr<FunkinSprite> child)
{
    sprite -> addChild(std::move(child));
}

void FunkinSprite::clearChildren()
{
    sprite -> clearChildren();
}

void FunkinSprite::setRenderSelf(bool value)
{
    sprite -> setRenderSelf(value);
}

FunkinGraphic::FunkinGraphic(const float width, const float height, const Color color)
{
    #if defined(PLATFORM_WINDOWS) || defined(PLATFORM_LINUX)
    sprite = std::make_unique<SDL3Graphic>(width, height, color);
    #endif
}

void FunkinGraphic::update(float elapsed)
{
    sprite -> update(elapsed);
}

void FunkinGraphic::render()
{
    sprite -> render();
}

SpriteRenderParams& FunkinGraphic::getParams()
{
    return sprite -> stuff;
}

float FunkinGraphic::getWidth()
{
    return sprite -> getWidth();
}

float FunkinGraphic::getHeight()
{
    return sprite -> getHeight();
}

void FunkinGraphic::addChild(std::unique_ptr<FunkinGraphic> child)
{
    sprite -> addChild(std::move(child));
}

void FunkinGraphic::clearChildren()
{
    sprite -> clearChildren();
}

void FunkinGraphic::setRenderSelf(bool value)
{
    sprite -> setRenderSelf(value);
}