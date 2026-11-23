#include <fstream>
#include <sstream>
#include <cctype>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <vector>
#include <cctype>

#include "jsonutil.hpp"

// json parser stuff that I totally didnt steal from Snake Engine

void skipJsonWhitespace(const std::string& src, size_t& p)
{
    while (p < src.size() && std::isspace(static_cast<unsigned char>(src[p])))
    {
        ++p;
    }
}

size_t findJsonKey(const std::string& src, const std::string& key, size_t startPos)
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

std::string jsonReadValue(const std::string& src, size_t p)
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

std::string jsonGetRaw(const std::string& src, const std::string& key, size_t startPos)
{
    size_t p = findJsonKey(src, key, startPos);
    if (p == std::string::npos) return "";

    std::string value = jsonReadValue(src, p);
    if (value.size() >= 2 && value.front() == '"' && value.back() == '"') value = value.substr(1, value.size() - 2);
    
    return value;
}

float fast_atof(const char* p, char** end)
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

float jsonGetFloat(const std::string& src, const std::string& key, float def)
{
    std::string value = jsonGetRaw(src, key);
    if (value.empty() || value == "null") return def;

    char* end = nullptr;
    float result = fast_atof(value.c_str(), &end);
    if (end == value.c_str() || *end != '\0') return def;

    return result;
}

int jsonGetInt(const std::string& src, const std::string& key, int def)
{
    std::string value = jsonGetRaw(src, key);
    if (value.empty() || value == "null") return def;

    char* end = nullptr;
    float result = fast_atof(value.c_str(), &end);
    if (end == value.c_str() || *end != '\0') return def;

    return static_cast<int>(result);
}

std::string jsonGetObject(const std::string& src, const std::string& key, size_t startPos)
{
    size_t p = findJsonKey(src, key, startPos);
    if (p == std::string::npos) return "";

    skipJsonWhitespace(src, p);
    if (p >= src.size() || src[p] != '{') return "";

    std::string result = jsonReadValue(src, p);
    return (!result.empty() && result.front() == '{') ? result : "";
}

std::string jsonGetArray(const std::string& src, const std::string& key, size_t startPos)
{
    size_t p = findJsonKey(src, key, startPos);
    if (p == std::string::npos) return "";

    skipJsonWhitespace(src, p);
    if (p >= src.size() || src[p] != '[') return "";

    std::string result = jsonReadValue(src, p);
    return (!result.empty() && result.front() == '[') ? result : "";
}

void forEachObject(const std::string& arr, std::function<void(const std::string&)> callback)
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