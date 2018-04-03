#include "engine.h"
#include <stdio.h>

Engine::Engine()
{
	window = new WindowHandle(500, 500, std::string("vulkan renderer"), this);
	renderer = new Renderer(std::string("vulkan renderer"), window);
	while (window->Update()) {
		renderer->Run();
    }
}


Engine::~Engine()
{
	delete renderer;
	delete window;
}
