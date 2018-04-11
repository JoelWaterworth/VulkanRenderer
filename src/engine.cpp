#include "engine.h"
#include <stdio.h>
#include <iostream>
#include <string>
#include "playerCamera.h"

Engine::Engine()
{
	window = WindowHandle(500, 500, std::string("vulkan renderer"), this);
#ifdef ALLOWINJECT



	std::string name;
	std::getline(std::cin, name);

	bool validation = false, debug = false;

	if (name.find("-v") != std::string::npos) {
		validation = true;
	}
	if ((name.find("-dr") != std::string::npos)) {
		debug = true;
	}

#ifdef _DEBUG
	validation = debug ? false : true;
#endif // _DEBUG
	renderer = Renderer(std::string("vulkan renderer"), window, validation, debug);
#endif // ALLOWINJECT
	renderer = Renderer(std::string("vulkan renderer"), &window, true, true);
	world = World();
	auto camera = PlayerCamera();
	camera.transform = Transform(glm::vec3(0.0f, 0.0f, -5.0f));
	world.addActor(&camera);
	clock_t begin = clock();
	while (window.Update()) {
		clock_t end = clock();
		double elapsed_secs = double(end - begin) / CLOCKS_PER_SEC;
		world.update(window.activeKeys, elapsed_secs);
		renderer.Run(&world);
    }
	renderer.destroy();
}


Engine::~Engine()
{
}

void Engine::updateGameState()
{

}
