#pragma once

#include "windowHandler.h"
#include "vulkanRender/renderer.h"
#include "actor.h"
#include "world.h"
#include <ctime>

class Engine
{
public:
	Engine();
	~Engine();
	clock_t timer;
	void updateGameState();
	World world;
	WindowHandle window;
	Renderer renderer;
};
