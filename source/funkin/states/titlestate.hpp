#include "../import.hpp"
#include "../../states/basestate.hpp"
#include "../graphics/atlastext.hpp"

class TitleState : public BaseState 
{
	public:
		void create() override;
		void update(float elapsed) override;
		void destroy() override;
		
		void stepHit(int step) override;
		void beatHit(int beat) override;
		void measureHit(int measure) override;

		TitleState();
		~TitleState();

		FunkinSprite* logoBl;
		FunkinSprite* gfDance;
		FunkinSprite* titleText;
		
		FunkinSound* freakyMenu;
		FunkinSound* confirmSound;
		
		void startIntro();
};