#include "engine.h"
#include <stdio.h>
#include <iostream>
#include <string>
#include "playerCamera.h"
#include "lightActor.h"

Engine::Engine()
{
	window = WindowHandle(1000, 1000, std::string("vulkan renderer"), this);
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
	auto camera = PlayerCamera(Transform(glm::vec3(0.0f, 0.0f, -12.0f), glm::vec3(0.0f, 0.0f, 0.f)));
	world.addActor(&camera);

	const float p = 15.0f;
	auto light1 = LightActor(Transform(glm::vec3(-p, -p, p*0.5f)));
	world.addActor(&light1);
	auto light2 = LightActor(Transform(glm::vec3(-p, p,  p*0.5f)));
	world.addActor(&light2);
	auto light3 = LightActor(Transform(glm::vec3( p, p, p*0.5f)));
	world.addActor(&light3);
	auto light4 = LightActor(Transform(glm::vec3( p, -p, p*0.5f)));
	world.addActor(&light4);
	
	clock_t begin = clock();
	while (window.Update()) {
		clock_t end = clock();
		double elapsed_secs = double(end - begin) / CLOCKS_PER_SEC;
		Event e = {};
		e.activeKeys = window.activeKeys;
		e.mousePos = window.deltaMousePos;
		e.bisLeftMouseDown = window.bisLMouse;
		world.update(e, elapsed_secs);
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
