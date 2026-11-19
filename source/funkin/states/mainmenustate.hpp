#include "../import.hpp"
#include "../../states/basestate.hpp"

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
};