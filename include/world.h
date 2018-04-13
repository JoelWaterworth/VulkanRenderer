#pragma once
#include "playerCamera.h"
#include "actor.h"
#include <vector>
#include <set>
#include "lightActor.h"

class World {
public:
	World();
	~World();

	double deltaTimer = 0.0;

	void update(std::set<char> keys, double deltaTime);

	void addActor(Actor* actor);

	std::vector<Actor*> actors = std::vector<Actor*>();

	Camera getCamera();

	std::vector<Light> getLights();
};