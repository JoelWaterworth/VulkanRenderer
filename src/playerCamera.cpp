#include "PlayerCamera.h"
#include "world.h"


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

void PlayerCamera::receiveInput(Event e, double deltaTime)
{
	float dt = (float)deltaTime;
	float const cameraSpeed = 0.001f;
	if (e.activeKeys.find('W') != e.activeKeys.end()) {
		transform.loction += glm::vec3(0.0f, 0.0f, 1.0f) * cameraSpeed * dt;
	}
	if (e.activeKeys.find('S') != e.activeKeys.end()) {
		transform.loction -= glm::vec3(0.0f, 0.0f, 1.0f) * cameraSpeed * dt;
	}
	if (e.activeKeys.find('A') != e.activeKeys.end()) {
		transform.loction -= glm::vec3(0.0f, 1.0f, 0.0f) * cameraSpeed * dt;
	}
	if (e.activeKeys.find('D') != e.activeKeys.end()) {
		transform.loction += glm::vec3(0.0f, 1.0f, 0.0f) * cameraSpeed * dt;
	}

	float const optionSpeed = 0.001f;

	if (e.activeKeys.find('Y') != e.activeKeys.end()) {
		exposure += optionSpeed * dt;
	}
	if (e.activeKeys.find('U') != e.activeKeys.end()) {
		exposure -= optionSpeed * dt;
	}

	if (e.activeKeys.find('G') != e.activeKeys.end()) {
		gamma += optionSpeed * dt;
	}
	if (e.activeKeys.find('H') != e.activeKeys.end()) {
		gamma -= optionSpeed * dt;
	}
	const float mouseSpeed = 0.1f;
	if (e.bisLeftMouseDown) {
		transform.rotation.x += e.mousePos.y * mouseSpeed;
		transform.rotation.y += e.mousePos.x * mouseSpeed;
	}

	exposure = abs(exposure);
	gamma = abs(gamma);
}
