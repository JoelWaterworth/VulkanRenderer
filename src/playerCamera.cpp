#include "PlayerCamera.h"



PlayerCamera::PlayerCamera()
{
}


PlayerCamera::~PlayerCamera()
{
}

void PlayerCamera::receiveInput(std::set<char> keys, double deltaTime)
{
	if (keys.find('w') != keys.end()) {
		printf("forward\n");
	}
	if (keys.find('s') != keys.end()) {
		printf("backwards\n");
	}
}
