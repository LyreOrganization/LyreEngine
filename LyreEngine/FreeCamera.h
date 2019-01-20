#pragma once

#include "Camera.h"

class FreeCamera : public Camera
{
public:
	FreeCamera();
	FreeCamera(const DirectX::XMFLOAT3& position,
			   const DirectX::XMFLOAT3& view,
			   const DirectX::XMFLOAT3& up);
	FreeCamera(const Camera& camera);
	virtual ~FreeCamera() override = default;

	void tilt(float angle);
	void pan(float angle);
	void roll(float angle);

	void moveAhead(float dist);
	void moveAside(float dist);
};