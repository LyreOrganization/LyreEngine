#include "stdafx.h"

#include "Camera.h"

using namespace std;
using namespace DirectX;

Camera::Camera() 
	: position{ -5.0f, 0.0f, 0.0f },
	view{ 1.0f, 0.0f, 0.0f },
	up{ 0.0f, 1.0f, 0.0f },
	m_fov(XM_PIDIV2 / 2)
{}

Camera::~Camera()
{}

XMVECTOR Camera::getRight() {
	return XMVector3Cross(XMLoadFloat3(&up), XMLoadFloat3(&view));
}

XMFLOAT4X4 Camera::getViewProj(float aspectWdivH) {
	XMFLOAT4X4 result;
	XMMATRIX viewMatrix = XMMatrixLookToLH(XMLoadFloat3(&position), XMLoadFloat3(&view), XMLoadFloat3(&up));
	XMMATRIX projectionMatrix = XMMatrixPerspectiveFovLH(m_fov, aspectWdivH, 100.f, 0.01f);
	XMStoreFloat4x4(&result, XMMatrixTranspose(viewMatrix*projectionMatrix));
	return result;
}

void Camera::tilt(float angle) {
	XMStoreFloat3(&view, XMVector3Transform(XMLoadFloat3(&view), XMMatrixRotationAxis(getRight(), angle)));
}

void Camera::pan(float angle) {
	XMStoreFloat3(&view, XMVector3Transform(XMLoadFloat3(&view), XMMatrixRotationAxis(-XMLoadFloat3(&up), angle)));
}

void Camera::moveForward(float dist) {
	XMStoreFloat3(&position, XMLoadFloat3(&position) + XMLoadFloat3(&view)*dist);
}

void Camera::moveBackward(float dist) {
	XMStoreFloat3(&position, XMLoadFloat3(&position) - XMLoadFloat3(&view)*dist);
}

void Camera::moveRight(float dist) {
	XMStoreFloat3(&position, XMLoadFloat3(&position) + getRight()*dist);
}

void Camera::moveLeft(float dist) {
	XMStoreFloat3(&position, XMLoadFloat3(&position) - getRight()*dist);
}