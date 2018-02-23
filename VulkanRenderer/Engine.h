#pragma once

#include "Window.h"
#include "vulkanRender/Renderer.h"

class Engine
{
public:
	Engine();
	~Engine();

	Window* window;
	Renderer* renderer;
};