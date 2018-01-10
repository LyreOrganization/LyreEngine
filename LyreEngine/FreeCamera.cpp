#include "stdafx.h"

#include "FreeCamera.h"

using namespace std;
using namespace DirectX;

FreeCamera::FreeCamera()
	: position { -5.0f, 0.0f, 0.0f },
	view { 1.0f, 0.0f, 0.0f },
	up { 0.0f, 1.0f, 0.0f },
	m_fov(XM_PIDIV2 / 2)
{}

FreeCamera::~FreeCamera() {}

XMVECTOR FreeCamera::getRight() {
	return XMVector3Cross(XMLoadFloat3(&up), XMLoadFloat3(&view));
}

XMFLOAT4X4 FreeCamera::getViewProj(float aspectWdivH) {
	XMFLOAT4X4 result;
	XMMATRIX viewMatrix = XMMatrixLookToLH(XMLoadFloat3(&position), XMLoadFloat3(&view), XMLoadFloat3(&up));
	XMMATRIX projectionMatrix = XMMatrixPerspectiveFovLH(m_fov, aspectWdivH, 100.f, 0.01f);
	XMStoreFloat4x4(&result, XMMatrixTranspose(viewMatrix*projectionMatrix));
	return result;
}

void FreeCamera::tilt(float angle) {
	XMMATRIX rotationMatrix = XMMatrixRotationAxis(getRight(), angle);
	XMStoreFloat3(&view, XMVector3Transform(XMLoadFloat3(&view), rotationMatrix));
	XMStoreFloat3(&up, XMVector3Transform(XMLoadFloat3(&up), rotationMatrix));
}

void FreeCamera::pan(float angle) {
	XMStoreFloat3(&view, XMVector3Transform(XMLoadFloat3(&view), XMMatrixRotationAxis(-XMLoadFloat3(&up), angle)));
}

void FreeCamera::roll(float angle) {
	XMStoreFloat3(&up, XMVector3Transform(XMLoadFloat3(&up), XMMatrixRotationAxis(XMLoadFloat3(&view), angle)));
}

void FreeCamera::moveForward(float dist) {
	XMStoreFloat3(&position, XMLoadFloat3(&position) + XMLoadFloat3(&view)*dist);
}

void FreeCamera::moveBackward(float dist) {
	XMStoreFloat3(&position, XMLoadFloat3(&position) - XMLoadFloat3(&view)*dist);
}

void FreeCamera::moveRight(float dist) {
	XMStoreFloat3(&position, XMLoadFloat3(&position) + getRight()*dist);
}

void FreeCamera::moveLeft(float dist) {
	XMStoreFloat3(&position, XMLoadFloat3(&position) - getRight()*dist);
}