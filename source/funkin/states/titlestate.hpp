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

		AtlasText* coolText1;
		AtlasText* coolText2;
		AtlasText* coolText3;

		FunkinSprite* ngSpr;

		FunkinGraphic* whiteFlash;
		
		FunkinSound* freakyMenu;
		FunkinSound* confirmSound;

		bool introSkipped;
		bool enterPressed;
		float daFlashAlpha = 0.0f;
		float timeTillMenu = 0.0f;
		
		void startIntro();
		void skipIntro();

		std::vector<std::string> curWacky;
		std::vector<std::vector<std::string>> getIntroTextShit();
};