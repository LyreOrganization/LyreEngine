#pragma once

class Camera {
protected:
	DirectX::XMFLOAT3 m_position;
	DirectX::XMFLOAT3 m_view;
	DirectX::XMFLOAT3 m_up;

	float m_fov;

	DirectX::XMVECTOR getRight();

public:
	Camera(DirectX::XMFLOAT3 position, 
		   DirectX::XMFLOAT3 view, 
		   DirectX::XMFLOAT3 up);
	virtual ~Camera();
	DirectX::XMFLOAT3 getPosition();
	DirectX::XMFLOAT4X4 calculateViewMatrix();
	DirectX::XMFLOAT4X4 calculateProjectionMatrix(float aspectWdivH);
	DirectX::XMFLOAT4X4 calculateViewProjMatrix(float aspectWdivH);

	bool setFOV(float fov = DirectX::XM_PIDIV4);
};