#pragma once
#include <memory>
#include <string>
#include <vector>
#include <map>

struct SpritemapStyle
{
    float redMultiplier = 1.0f;
    float greenMultiplier = 1.0f;
    float blueMultiplier = 1.0f;
    float alphaMultiplier = 1.0f;

    float redOffset = 0.0f;
    float greenOffset = 0.0f;
    float blueOffset = 0.0f;
    float alphaOffset = 0.0f;

    std::string blend;
    bool hasBlend = false;
    bool hasColor = false;
};

struct SpritemapPiece
{
    std::string name;
    int x;
    int y;
    int w;
    int h;
    bool rotated;
};

struct AffineMatrix
{
    float a = 1;
    float b = 0;
    float c = 0;
    float d = 1;
    float tx = 0;
    float ty = 0;
};

struct Pivot
{
    float x = 0;
    float y = 0;
};

struct AtlasSpriteInstance
{
    std::string pieceName;
    AffineMatrix mx;
};

struct SymbolInstance
{
    std::string symbolName;
    std::string loopType;
    int firstFrame;
    AffineMatrix mx;
    Pivot trp;
    SpritemapStyle style;
};

struct SpritemapElement
{
    bool isAtlasSprite;
    AtlasSpriteInstance asi;
    SymbolInstance si;
};

struct SpritemapKeyframe
{
    int index;
    int duration;
    std::vector<SpritemapElement> elements;
    std::string blend;
    bool hasBlend = false;
};

struct SpritemapLayer
{
    std::string name;
    std::vector<SpritemapKeyframe> keyframes;
};

struct SpritemapSymbol
{
    std::string name;
    std::vector<SpritemapLayer> layers;

    int duration() const;
    const SpritemapKeyframe* getKeyframe(int layerIdx, int frame) const;
};

struct SpritemapAnimation
{
    std::string name;
    int startFrame;
    int duration;
    std::string symbolName;
    std::string loopType;
};

struct SpritemapData
{
    std::map<std::string, SpritemapPiece> atlas;
    std::map<std::string, SpritemapSymbol> symbols;
    std::vector<SpritemapAnimation> animations;
    std::string rootSymbolName;

    float canvasW = 1280;
    float canvasH = 720;
    int frameRate = 24;
};

class SpritemapParser
{
    public:
        static bool parse(const std::string& atlasJsonPath, const std::string& animJsonPath, SpritemapData& out);

    private:
        static bool parseAtlas(const std::string& path, SpritemapData& out);
        static bool parseAnimation(const std::string& path, SpritemapData& out);
};

struct SpriteRect 
{
    public:
        float x = 0.0f;
        float y = 0.0f;
        float w = 0.0f;
        float h = 0.0f;
};

struct AtlasFrame
{
    std::string name;
    int index = 0;

    float x = 0.0f;
    float y = 0.0f;
    float width = 0.0f;
    float height = 0.0f;

    float frameX = 0.0f;
    float frameY = 0.0f;
    float frameWidth = 0.0f;
    float frameHeight = 0.0f;
};

struct Animation
{
    std::string name;
    std::vector<AtlasFrame> frames;

    float fps = 24.0f;
    bool looped = false;
    bool flipX = false;
    bool flipY = false;
};

struct Color
{
    float r = 1.0f;
    float g = 1.0f;
    float b = 1.0f;
};

class SpriteRenderParams
{
    public:
        float x = 0.0f;
        float y = 0.0f;
        float width = 0.0f;
        float height = 0.0f;

        float scaleX = 1.0f;
        float scaleY = 1.0f;

        float skewX = 0.0f;
        float skewY = 0.0f;

        float alpha = 1.0f;
        float angle = 0.0f;
        float originX = 0.0f;
        float originY = 0.0f;
        bool flipX = false;
        bool flipY = false;

        bool visible = true;
        Color color;
        std::string blend = "none";
};

class FunkinSprite;

class BaseSprite
{
    protected:
		SpriteRect drect;
		SpriteRect srect;

        std::vector<Animation> anims;

        int curAnim = 0;
        int curFrame = 0;

        float animTimer = 0.0f;
        bool animPlaying = false;

        float animFPS = 24.0f;
        bool animLoop = true;

        std::vector<std::unique_ptr<FunkinSprite>> children;
        bool renderSelf = true;

    public:
        BaseSprite() = default;
        virtual void create(const std::string& path, const std::string& type = "none") = 0;
        virtual ~BaseSprite();

        SpritemapData spritemapData;
		SpriteRenderParams stuff;
        std::string spriteType;
		
        float frameX = 0.0f;
        float frameY = 0.0f;
        float frameWidth = 0.0f;
        float frameHeight = 0.0f;

        float getWidth();
        float getHeight();
        float getAtlasMaxHeight() const;

        void update(float elapsed);
        virtual void render();

        std::string getAnimName(const std::string& frameName, int& frameIndex);
        void loadAtlas(const std::string& path);
        bool loadSpritemap(const std::string& path);

        void addAnimFrames(const std::string& name, const std::string& prefix, const std::vector<int>& indices, float fps, bool looped, bool flipX, bool flipY);

        void addAnim(const std::string& name, const std::string& prefix, float fps = 24.0f, bool looped = false, bool flipX = false, bool flipY = false);
        void addAnim(const std::string& name, const std::string& prefix, const std::vector<int>& indices, float fps = 24.0f, bool looped = false, bool flipX = false, bool flipY = false);
        
        void addSpritemapAnim(const std::string& name, const std::string& symbolName, const std::vector<int>& indices = {}, float fps = 24.0f, bool loop = true);

        void playAnim(const std::string& name);

        std::vector<std::string> getAtlasFrameNames() const;

        void drawSymbol(const std::string& symbolName, int logicalFrame, const AffineMatrix& parentMX, float depth, int recursionDepth = 0) const;
        void drawPiece(const std::string& pieceName, const AffineMatrix& mx, float depth) const;

        void addChild(std::unique_ptr<FunkinSprite> child);
        void clearChildren();

        void setRenderSelf(bool value)
        {
            renderSelf = value;
        }
};

class FunkinSprite
{
    friend class SpriteManager;

    private:
        std::unique_ptr<BaseSprite> sprite;

    public:
        FunkinSprite(const std::string& path, const std::string& type = "none");
        virtual ~FunkinSprite() = default;

        void update(float elapsed);
        void render();

        void addAnim(const std::string& name, const std::string& prefix, float fps = 24.0f, bool looped = false, bool flipX = false, bool flipY = false);
        void addAnim(const std::string& name, const std::string& prefix, const std::vector<int>& indices, float fps = 24.0f, bool looped = false, bool flipX = false, bool flipY = false);

        void addSpritemapAnim(const std::string& name, const std::string& symbolName, const std::vector<int>& indices = {}, float fps = 24.0f, bool loop = false);

        void playAnim(const std::string& name);

        std::vector<std::string> getAtlasFrameNames() const;

        SpriteRenderParams& getParams();

        float getWidth();
        float getHeight();
        float getAtlasMaxHeight() const;

        void addChild(std::unique_ptr<FunkinSprite> child);
        void clearChildren();

        void setRenderSelf(bool value);
};