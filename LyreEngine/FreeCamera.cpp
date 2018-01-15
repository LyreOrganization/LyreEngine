#include "stdafx.h"

#include "FreeCamera.h"

using namespace std;
using namespace DirectX;

FreeCamera::FreeCamera()
	: m_position { -5.0f, 0.0f, 0.0f },
	m_view { 1.0f, 0.0f, 0.0f },
	m_up { 0.0f, 1.0f, 0.0f },
	m_fov(XM_PIDIV2 / 2)
{}

FreeCamera::~FreeCamera() {}

XMVECTOR FreeCamera::getRight() {
	return XMVector3Cross(XMLoadFloat3(&m_up), XMLoadFloat3(&m_view));
}

XMFLOAT3 FreeCamera::getPosition() {
	return m_position;
}

XMFLOAT4X4 FreeCamera::getView() {
	XMFLOAT4X4 result;
	XMStoreFloat4x4(&result, XMMatrixTranspose(XMMatrixLookToLH(XMLoadFloat3(&m_position), XMLoadFloat3(&m_view), XMLoadFloat3(&m_up))));
	return result;
}

DirectX::XMFLOAT4X4 FreeCamera::getProjection(float aspectWdivH) {
	XMFLOAT4X4 result;
	XMStoreFloat4x4(&result, XMMatrixTranspose(XMMatrixPerspectiveFovLH(m_fov, aspectWdivH, 0.01f, 100.f)));
	return result;
}

DirectX::XMFLOAT4X4 FreeCamera::getViewProj(float aspectWdivH) {
	XMFLOAT4X4 result;
	XMStoreFloat4x4(&result, XMMatrixTranspose(XMMatrixLookToLH(XMLoadFloat3(&m_position), XMLoadFloat3(&m_view), XMLoadFloat3(&m_up))*
											   XMMatrixPerspectiveFovLH(m_fov, aspectWdivH, 0.01f, 100.f)));
	return result;
}

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