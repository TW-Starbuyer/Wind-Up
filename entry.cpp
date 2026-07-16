#define SDL_MAIN_HANDLED

#include <SDL3/SDL.h>

#include "src/common/app_entry.hpp"

int main() 
{
	WINDUP_ENGINE windup_engine;
	WINDUP_App* app = create_app();
	
	windup_engine.load_app(app);
	windup_engine.run();
	windup_engine.deinit();

	delete app;
	return 0;
}