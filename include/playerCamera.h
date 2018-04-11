#pragma once
#include "actor.h"

struct Camera {
	Transform transform;

	Camera(Transform tran = Transform()) : transform(tran) {};
};

class PlayerCamera : public Actor {
public:
	PlayerCamera();
	~PlayerCamera();

	virtual void receiveInput(std::set<char> keys, double deltaTime);
};

