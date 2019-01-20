#include "stdafx.h"

#include "Camera.h"

using namespace std;
using namespace DirectX;

Camera::Camera(const XMFLOAT3& position,
			   const XMFLOAT3& view,
			   const XMFLOAT3& up)
	: m_position(position), m_view(view), m_up(up), m_fov(XM_PIDIV4) {
	XMVECTOR vView = XMVector3Normalize(XMLoadFloat3(&m_view));
	XMStoreFloat3(&m_view, vView);
	XMVECTOR vUp = XMLoadFloat3(&m_up);
	XMVector3ComponentsFromNormal(&XMVECTOR(), &vUp, XMLoadFloat3(&m_up), vView);
	XMStoreFloat3(&m_up, XMVector3Normalize(vUp));
}

Camera::Camera()
	: m_position({ 0.f, 0.f, -1.f }),
	m_view({ 0.f, 0.f, 1.f }),
	m_up({ 0.f, 1.f, 0.f }),
	m_fov(XM_PIDIV4) {}

Camera::Camera(const Camera& camera)
	: m_position(camera.m_position), m_view(camera.m_view), m_up(camera.m_up), 
	m_fov(camera.m_fov) {}

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
	XMStoreFloat4x4(&result, XMMatrixTranspose(XMMatrixPerspectiveFovLH(m_fov, aspectWdivH, 0.005f, 2000.f)));
	return result;
}

DirectX::XMFLOAT4X4 Camera::calculateViewProjMatrix(float aspectWdivH) {
	XMFLOAT4X4 result;
	XMStoreFloat4x4(&result, XMMatrixTranspose(XMMatrixLookToLH(XMLoadFloat3(&m_position), XMLoadFloat3(&m_view), XMLoadFloat3(&m_up))*
											   XMMatrixPerspectiveFovLH(m_fov, aspectWdivH, 0.005f, 2000.f)));
	return result;
}

bool Camera::setFOV(float fov) {
	if (fov < XM_PIDIV4 || fov > XM_PIDIV4 * 3)
		return false;
	m_fov = fov;
	return true;
}