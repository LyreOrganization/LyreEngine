#include "stdafx.h"

#include "FreeCamera.h"

using namespace std;
using namespace DirectX;

FreeCamera::FreeCamera() : Camera() {}

FreeCamera::FreeCamera(const XMFLOAT3& position,
					   const XMFLOAT3& view,
					   const XMFLOAT3& up)
	: Camera(position, view, up) {}

FreeCamera::FreeCamera(const Camera& camera)
	: Camera(camera) {}

void FreeCamera::tilt(float angle) {
	XMVECTOR quaternion = XMQuaternionRotationAxis(getRight(), angle);
	XMStoreFloat3(&m_view, XMVector3Rotate(XMLoadFloat3(&m_view), quaternion));
	XMStoreFloat3(&m_up, XMVector3Rotate(XMLoadFloat3(&m_up), quaternion));
}

void FreeCamera::pan(float angle) {
	XMStoreFloat3(&m_view, XMVector3Transform(XMLoadFloat3(&m_view), XMMatrixRotationAxis(-XMLoadFloat3(&m_up), angle)));
}

void FreeCamera::roll(float angle) {
	XMStoreFloat3(&m_up, XMVector3Transform(XMLoadFloat3(&m_up), XMMatrixRotationAxis(XMLoadFloat3(&m_view), angle)));
}

void FreeCamera::moveAhead(float dist) {
	XMStoreFloat3(&m_position, XMLoadFloat3(&m_position) + XMLoadFloat3(&m_view)*dist);
}

void FreeCamera::moveAside(float dist) {
	XMStoreFloat3(&m_position, XMLoadFloat3(&m_position) + getRight()*dist);
}
