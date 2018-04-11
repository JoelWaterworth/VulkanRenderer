#pragma once
#include <set>
#include <glm\glm.hpp>

struct Transform {
	glm::vec3 loction;
	glm::vec3 rotation;
	glm::vec3 scale;

	//Transform(glm::vec3 loc) : loction(loc) {};

	Transform(
		glm::vec3 loc = glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3 rot = glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3 scal = glm::vec3(1.0f, 1.0f, 1.0f)
	) : loction(loc), rotation(rot), scale(scal) {};
};

class Actor {
public:
	Actor();
	~Actor();

	Transform transform;

	virtual void receiveInput(std::set<char> keys, double deltaTime);
};