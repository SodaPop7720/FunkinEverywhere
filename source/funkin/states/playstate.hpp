#pragma once

#include <string>
#include <vector>

#include "../import.hpp"

class PlayState : public BaseState 
{
	public:
		void create() override;
		void update(float elapsed) override;
		void destroy() override;

		void stepHit(int step) override;
		void beatHit(int beat) override;
		void measureHit(int measure) override;

		PlayState();
		~PlayState();
};