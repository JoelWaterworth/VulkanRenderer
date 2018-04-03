#pragma once

#include "windowHandler.h"
#include "vulkanRender/renderer.h"

class Engine
{
public:
	Engine();
	~Engine();

	WindowHandle* window;
	Renderer* renderer;
};
