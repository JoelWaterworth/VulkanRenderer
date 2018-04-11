#include "PlayerCamera.h"



PlayerCamera::PlayerCamera()
{
}


PlayerCamera::~PlayerCamera()
{
}

void PlayerCamera::receiveInput(std::set<char> keys, double deltaTime)
{
	if (keys.find('W') != keys.end()) {
		transform.loction += glm::vec3(0.0f, 0.0f, 0.1f * deltaTime);
	}
	if (keys.find('S') != keys.end()) {
		transform.loction += glm::vec3(0.0f, 0.0f, -0.1f * deltaTime);
	}
	if (keys.find('A') != keys.end()) {
		transform.loction += glm::vec3(0.1f * deltaTime, 0.0f, 0.0f);
	}
	if (keys.find('D') != keys.end()) {
		transform.loction += glm::vec3(-0.1f * deltaTime, 0.0f, 0.0f);
	}
}
