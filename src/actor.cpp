#include "actor.h"

Actor::Actor()
{

}

glm::vec3 Actor::getFowardVector() {
	glm::mat4 rot = glm::rotate(glm::mat4(1.0f), glm::radians(transform.rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
	rot = glm::rotate(rot, glm::radians(transform.rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
	rot = glm::rotate(rot, glm::radians(transform.rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
	return rot * glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
}

glm::vec3 Actor::getRightwardVector() {
	glm::mat4 rot = glm::rotate(glm::mat4(1.0f), glm::radians(transform.rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
	rot = glm::rotate(rot, glm::radians(transform.rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
	rot = glm::rotate(rot, glm::radians(transform.rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
	return rot * glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
}

Actor::~Actor()
{
}

void Actor::receiveInput(Event e, double deltaTime)
{
}
