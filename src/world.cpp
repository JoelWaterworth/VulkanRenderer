#include "world.h"
#include "actor.h"


World::World() {

}

World::~World() {
	for (auto actor : actors) {
		//delete actor;
	}
}

void World::update(std::set<char> keys, double deltaTime){
	for (auto actor : actors) {
		actor->receiveInput(keys, deltaTime);
	}
}

void World::addActor(Actor * actor)
{
	actors.push_back(actor);
}

Camera World::getCamera()
{
	for (auto actor : actors) {
		PlayerCamera *ptr = dynamic_cast<PlayerCamera*>(actor);
		if (ptr) {
			return ptr->getCamera();
		}
	}
	return Camera();
}

std::vector<Light> World::getLights() {
	std::vector<Light> lights;
	for (auto actor : actors) {
		LightActor *ptr = dynamic_cast<LightActor*>(actor);
		if (ptr) {
			lights.push_back(ptr->getLight());
		}
	}
	return lights;
}