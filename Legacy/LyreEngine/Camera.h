#pragma once

class Camera {
protected:
	DirectX::XMFLOAT3 m_position;
	DirectX::XMFLOAT3 m_view;
	DirectX::XMFLOAT3 m_up;

	float m_fov;

	DirectX::XMVECTOR getRight();

	Camera();

public:
	Camera(const DirectX::XMFLOAT3& position,
		   const DirectX::XMFLOAT3& view,
		   const DirectX::XMFLOAT3& up);
	Camera(const Camera& camera);
	virtual ~Camera() = default;
	DirectX::XMFLOAT3 getPosition();
	DirectX::XMFLOAT4X4 calculateViewMatrix();
	DirectX::XMFLOAT4X4 calculateProjectionMatrix(float aspectWdivH);
	virtual DirectX::XMFLOAT4X4 calculateViewProjMatrix(float aspectWdivH);

	bool setFOV(float fov = DirectX::XM_PIDIV4);
};