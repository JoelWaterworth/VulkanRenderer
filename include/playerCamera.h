#pragma once
#include "actor.h"

struct Camera {
	Transform transform;
	float exposure = 0.0f;
	float gamma = 0.0f;
	Camera(Transform tran = Transform()) : transform(tran) {};
};

class PlayerCamera : public Actor {
public:
	PlayerCamera();
	PlayerCamera(Transform trans) : Actor(trans) {};
	~PlayerCamera();
	float exposure = 4.5f;
	float gamma = 2.2f;

	Camera getCamera();

	virtual void receiveInput(Event e, double deltaTime);
};

