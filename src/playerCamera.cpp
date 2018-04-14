#include "PlayerCamera.h"



PlayerCamera::PlayerCamera()
{
}


PlayerCamera::~PlayerCamera()
{
}

Camera PlayerCamera::getCamera() {
	Camera camera = Camera(transform);
	camera.gamma = gamma;
	camera.exposure = exposure;
	return camera;
}

void PlayerCamera::receiveInput(std::set<char> keys, double deltaTime)
{
	float dt = (float)deltaTime;
	float const cameraSpeed = 0.01f;
	if (keys.find('W') != keys.end()) {
		transform.loction += glm::vec3(0.0f, 0.0f, -cameraSpeed * dt);
	}
	if (keys.find('S') != keys.end()) {
		transform.loction += glm::vec3(0.0f, 0.0f, cameraSpeed * dt);
	}
	if (keys.find('A') != keys.end()) {
		transform.loction += glm::vec3(-cameraSpeed * dt, 0.0f, 0.0f);
	}
	if (keys.find('D') != keys.end()) {
		transform.loction += glm::vec3(cameraSpeed * dt, 0.0f, 0.0f);
	}

	float const optionSpeed = 0.001f;

	if (keys.find('Y') != keys.end()) {
		exposure += optionSpeed * dt;
	}
	if (keys.find('U') != keys.end()) {
		exposure -= optionSpeed * dt;
	}

	if (keys.find('G') != keys.end()) {
		gamma += optionSpeed * dt;
	}
	if (keys.find('H') != keys.end()) {
		gamma -= optionSpeed * dt;
	}

	exposure = abs(exposure);
	gamma = abs(gamma);
}
