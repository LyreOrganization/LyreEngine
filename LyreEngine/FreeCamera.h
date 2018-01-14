#pragma once

class FreeCamera
{
public:
	DirectX::XMFLOAT3 m_position;
	DirectX::XMFLOAT3 m_view;
	DirectX::XMFLOAT3 m_up;

protected:
	DirectX::XMFLOAT4X4 m_projection;
	float m_fov;

	DirectX::XMVECTOR getRight();

public:
	FreeCamera();
	virtual ~FreeCamera();
	DirectX::XMFLOAT3 getPosition();
	DirectX::XMFLOAT4X4 getView();
	DirectX::XMFLOAT4X4 getProjection(float aspectWdivH);

	void tilt(float angle);
	void pan(float angle);
	void roll(float angle);

	void moveForward(float dist);
	void moveBackward(float dist);
	void moveRight(float dist);
	void moveLeft(float dist);
};