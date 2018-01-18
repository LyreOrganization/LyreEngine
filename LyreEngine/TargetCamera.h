#pragma once

#include "Camera.h"

class TargetCamera : public Camera {
protected:
	DirectX::XMFLOAT3 m_target;
	float m_radius;

public:
	TargetCamera(const DirectX::XMFLOAT3 &position,
				 const DirectX::XMFLOAT3 &target, float radius);
	TargetCamera(const Camera& camera, const DirectX::XMFLOAT3 &target, float radius);
	virtual ~TargetCamera() override;
	void approach(float percentage);
	void rotateAroundHorizontally(float angle);
	void rotateAroundVertically(float angle);
	void spin(float angle);
	void tilt(float angle);
};