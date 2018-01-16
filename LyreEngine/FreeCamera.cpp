#include "stdafx.h"

#include "FreeCamera.h"

using namespace std;
using namespace DirectX;

FreeCamera::FreeCamera(XMFLOAT3 position, XMFLOAT3 view, XMFLOAT3 up)
	: Camera(position, view, up) {}

FreeCamera::~FreeCamera() {}

void FreeCamera::tilt(float angle) {
	XMMATRIX rotationMatrix = XMMatrixRotationAxis(getRight(), angle);
	XMStoreFloat3(&m_view, XMVector3Transform(XMLoadFloat3(&m_view), rotationMatrix));
	XMStoreFloat3(&m_up, XMVector3Transform(XMLoadFloat3(&m_up), rotationMatrix));
}

void FreeCamera::pan(float angle) {
	XMStoreFloat3(&m_view, XMVector3Transform(XMLoadFloat3(&m_view), XMMatrixRotationAxis(-XMLoadFloat3(&m_up), angle)));
}

void FreeCamera::roll(float angle) {
	XMStoreFloat3(&m_up, XMVector3Transform(XMLoadFloat3(&m_up), XMMatrixRotationAxis(XMLoadFloat3(&m_view), angle)));
}

void FreeCamera::moveForward(float dist) {
	XMStoreFloat3(&m_position, XMLoadFloat3(&m_position) + XMLoadFloat3(&m_view)*dist);
}

void FreeCamera::moveBackward(float dist) {
	XMStoreFloat3(&m_position, XMLoadFloat3(&m_position) - XMLoadFloat3(&m_view)*dist);
}

void FreeCamera::moveRight(float dist) {
	XMStoreFloat3(&m_position, XMLoadFloat3(&m_position) + getRight()*dist);
}

void FreeCamera::moveLeft(float dist) {
	XMStoreFloat3(&m_position, XMLoadFloat3(&m_position) - getRight()*dist);
}