#pragma once

#include <string>
#include <vector>

#include "../import.hpp"

class MainMenuState : public BaseState 
{
	public:
		void create() override;
		void update(float elapsed) override;
		void destroy() override;

		void stepHit(int step) override;
		void beatHit(int beat) override;
		void measureHit(int measure) override;

		MainMenuState();
		~MainMenuState();

		std::vector<std::string> optionShit = {
			"freeplay",
			"credits",
			"options"
		};
		
		FunkinSprite* bg;
		FunkinSprite* menuItemSprites;
		std::vector<FunkinSprite*> menuItems;

		int curSelected = 0;

		void changeSelection(int silly);
};