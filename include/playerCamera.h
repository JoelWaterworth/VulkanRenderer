#pragma once
#include "actor.h"

struct Camera {
	Transform transform;

	Camera(Transform tran = Transform()) : transform(tran) {};
};

class PlayerCamera : public Actor {
public:
	PlayerCamera();
	PlayerCamera(Transform trans) : Actor(trans) {};
	~PlayerCamera();

	virtual void receiveInput(std::set<char> keys, double deltaTime);
};

