#include "world.h"
#include "actor.h"


World::World() {

}

World::~World() {
	for (auto actor : actors) {
		delete actor;
	}
}

void World::update(std::set<char> keys){
	for (auto actor : actors) {
		actor->receiveInput(keys, deltaTimer);
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
			return { ptr->transform };
		}
	}
	return Camera();
}
