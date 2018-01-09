#pragma once

class Camera
{
public:
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT3 view;
	DirectX::XMFLOAT3 up;

protected:
	DirectX::XMFLOAT4X4 m_projection;
	float m_fov;

	DirectX::XMVECTOR getRight();

public:
	Camera();
	virtual ~Camera();
	DirectX::XMFLOAT4X4 getViewProj(float aspectWdivH);
	void tilt(float angle);
	void pan(float angle);

	void moveForward(float dist);
	void moveBackward(float dist);
	void moveRight(float dist);
	void moveLeft(float dist);
};