#pragma once

#include <string>
#include <vector>

#include "../import.hpp"

class Character : public FunkinSprite
{
    public:
        Character(const std::string& name);
};