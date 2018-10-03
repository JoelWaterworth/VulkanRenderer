#pragma once
#include <set>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

struct Event {
	std::set<char> activeKeys;
	glm::vec2 mousePos;
	bool bisLeftMouseDown;
};

struct Transform {
	glm::vec3 loction;
	glm::vec3 rotation;
	glm::vec3 scale;

	//Transform(glm::vec3 loc) : loction(loc) {};

	Transform(
		glm::vec3 loc = glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3 rot = glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3 scal = glm::vec3(1.0f, 1.0f, 1.0f)
	) : loction(loc), rotation(rot), scale(scal) {}

	glm::mat4 rotationToMatrix() {
		glm::mat4 rot = glm::rotate(glm::mat4(1.0f), glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
		rot = glm::rotate(rot, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
		rot = glm::rotate(rot, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
		return rot;
	}
};

class Actor {
public:
	Actor();
	Actor(Transform tran) : transform(tran) {};
	~Actor();

	Transform transform;
	glm::vec3 getFowardVector();
	glm::vec3 getRightwardVector();
	virtual void receiveInput(Event e, double deltaTime);
};
