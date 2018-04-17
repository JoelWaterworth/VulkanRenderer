#pragma once
#include "actor.h"
#include <glm/glm.hpp>

struct Light {
	glm::vec4 position;
	glm::vec3 color;
	float radius;
	Light(glm::vec4 pos, glm::vec3 c, float r = 10.0f) :
		position(pos), color(c), radius(r) {};
	Light() : position(glm::vec4()), color(glm::vec3()), radius(10.0f) {};
};

class LightActor : public Actor
{
public:
	glm::vec3 colour = glm::vec3(150.0f);
	float radius = 0.0f;

	inline Light getLight() {
		return Light(glm::vec4(transform.loction, 1.0f), colour, radius);
	}

	LightActor(Transform tran) : Actor(tran) {};
	LightActor();
	~LightActor();
};

