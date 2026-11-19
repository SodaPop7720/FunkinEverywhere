class BaseState 
{
	public:
		bool isInitialized = false;

		virtual void create() = 0;
		virtual void update(float elapsed) = 0;
		virtual void destroy() = 0;

		virtual void stepHit(int step) = 0;
		virtual void beatHit(int beat) = 0;
		virtual void measureHit(int measure) = 0;

		virtual ~BaseState();
};

class StateManager 
{
	private:
		static BaseState *curState;

	public:
		static BaseState *prevState;
		static void changeState(BaseState *state);
		static void update(float elapsed);
		static void destroy();

		static void stepHit(int step);
		static void beatHit(int beat);
		static void measureHit(int measure);
};

extern StateManager states;