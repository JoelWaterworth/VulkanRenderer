#include "engine.h"
#include <stdio.h>
#include <iostream>
#include <string>

Engine::Engine()
{
	window = new WindowHandle(500, 500, std::string("vulkan renderer"), this);
	std::string name;
	std::getline(std::cin, name);

	bool validation = false, debug = false;

	if (name.find("-v") != std::string::npos) {
//		validation = true;
	}
	if ((name.find("-dr") != std::string::npos)) {
		debug = true;
	}

#ifdef _DEBUG
	//validation = debug ? false : true;
#endif // _DEBUG


	renderer = new Renderer(std::string("vulkan renderer"), window, validation, debug);
	while (window->Update()) {
		renderer->Run();
    }
}


Engine::~Engine()
{
	delete renderer;
	delete window;
}
