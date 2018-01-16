#pragma once

#include "Camera.h"

class FreeCamera : public Camera
{
public:
	FreeCamera(DirectX::XMFLOAT3 position,
		   DirectX::XMFLOAT3 view,
		   DirectX::XMFLOAT3 up);
	virtual ~FreeCamera() override;

	void tilt(float angle);
	void pan(float angle);
	void roll(float angle);

	void moveForward(float dist);
	void moveBackward(float dist);
	void moveRight(float dist);
	void moveLeft(float dist);
};