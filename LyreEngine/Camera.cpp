#include "stdafx.h"

#include "Camera.h"

using namespace std;
using namespace DirectX;

Camera::Camera(XMFLOAT3 position, XMFLOAT3 view, XMFLOAT3 up)
	: m_position(position), m_view(view), m_up(up), m_fov(XM_PIDIV4) {}

Camera::~Camera() {}

XMVECTOR Camera::getRight() {
	return XMVector3Cross(XMLoadFloat3(&m_up), XMLoadFloat3(&m_view));
}

XMFLOAT3 Camera::getPosition() {
	return m_position;
}

XMFLOAT4X4 Camera::calculateViewMatrix() {
	XMFLOAT4X4 result;
	XMStoreFloat4x4(&result, XMMatrixTranspose(XMMatrixLookToLH(XMLoadFloat3(&m_position), XMLoadFloat3(&m_view), XMLoadFloat3(&m_up))));
	return result;
}

DirectX::XMFLOAT4X4 Camera::calculateProjectionMatrix(float aspectWdivH) {
	XMFLOAT4X4 result;
	XMStoreFloat4x4(&result, XMMatrixTranspose(XMMatrixPerspectiveFovLH(m_fov, aspectWdivH, 0.01f, 100.f)));
	return result;
}

DirectX::XMFLOAT4X4 Camera::calculateViewProjMatrix(float aspectWdivH) {
	XMFLOAT4X4 result;
	XMStoreFloat4x4(&result, XMMatrixTranspose(XMMatrixLookToLH(XMLoadFloat3(&m_position), XMLoadFloat3(&m_view), XMLoadFloat3(&m_up))*
											   XMMatrixPerspectiveFovLH(m_fov, aspectWdivH, 0.01f, 100.f)));
	return result;
}

bool Camera::setFOV(float fov) {
	if (fov < XM_PIDIV4 || fov > XM_PIDIV4 * 3)
		return false;
	m_fov = fov;
	return true;
}