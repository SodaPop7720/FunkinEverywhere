#pragma once

#include <functional>
#include <string>

void skipJsonWhitespace(const std::string& src, size_t& p);
size_t findJsonKey(const std::string& src, const std::string& key, size_t startPos = 0);
std::string jsonReadValue(const std::string& src, size_t p);
std::string jsonGetRaw(const std::string& src, const std::string& key, size_t startPos = 0);
float fast_atof(const char* p, char** end = nullptr);
float jsonGetFloat(const std::string& src, const std::string& key, float def = 0.0f);
int jsonGetInt(const std::string& src, const std::string& key, int def = 0);
std::string jsonGetObject(const std::string& src, const std::string& key, size_t startPos = 0);
std::string jsonGetArray(const std::string& src, const std::string& key, size_t startPos = 0);
void forEachObject(const std::string& arr, std::function<void(const std::string&)> callback);