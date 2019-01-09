#include "stdafx.h"

#include "TargetCamera.h"

using namespace DirectX;

namespace {
	const float MAX_TILT = XM_PIDIV2 - 0.001f;
}

TargetCamera::TargetCamera(const XMFLOAT3& position,
						   const XMFLOAT3& target, float radius)
	: Camera(), m_target(target), m_radius(radius) {

	m_position = position;
	XMVECTOR vTarget = XMLoadFloat3(&m_target) - XMLoadFloat3(&position);
	float distance = (XMVectorGetX(XMVector3Length(vTarget)) - m_radius);
	if (distance < 0)
		XMStoreFloat3(&m_position, XMLoadFloat3(&m_position) - XMVector3Normalize(vTarget)*distance);

	XMVECTOR vView = XMVector3Normalize(XMLoadFloat3(&m_target) - XMLoadFloat3(&m_position));
	XMStoreFloat3(&m_view, vView);
	XMVECTOR vUp = XMLoadFloat3(&m_up);
	XMVector3ComponentsFromNormal(&XMVECTOR(), &vUp, XMLoadFloat3(&m_up), vView);
	XMStoreFloat3(&m_up, XMVector3Normalize(vUp));
}

TargetCamera::TargetCamera(const Camera& camera, 
						   const DirectX::XMFLOAT3 &target, float radius)
	:Camera(camera), m_target(target), m_radius(radius) {

	XMVECTOR vTarget = XMLoadFloat3(&m_target) - XMLoadFloat3(&m_position);
	float distance = (XMVectorGetX(XMVector3Length(vTarget)) - m_radius);
	if (distance < 0)
		XMStoreFloat3(&m_position, XMLoadFloat3(&m_position) - XMVector3Normalize(vTarget)*distance);

	XMVECTOR vView = XMVector3Normalize(XMLoadFloat3(&m_target) - XMLoadFloat3(&m_position));
	XMStoreFloat3(&m_view, vView);
	XMVECTOR vUp = XMLoadFloat3(&m_up);
	XMVector3ComponentsFromNormal(&XMVECTOR(), &vUp, XMLoadFloat3(&m_up), vView);
	XMStoreFloat3(&m_up, XMVector3Normalize(vUp));
}

TargetCamera::~TargetCamera() {}

void TargetCamera::approach(float percentage) {
	XMVECTOR vTarget = XMLoadFloat3(&m_target) - XMLoadFloat3(&m_position);
	float moveDistance = (XMVectorGetX(XMVector3Length(vTarget)) - m_radius) * percentage / 100.f;
	XMStoreFloat3(&m_position, XMLoadFloat3(&m_position) + XMVector3Normalize(vTarget)*moveDistance);
}

void TargetCamera::rotateAroundHorizontally(float angle) {
	XMVECTOR quaternion = XMQuaternionRotationNormal(XMVector3Normalize(getRight()), angle);
	XMStoreFloat3(&m_position, XMLoadFloat3(&m_target) + XMVector3Rotate(
		XMLoadFloat3(&m_position) - XMLoadFloat3(&m_target), quaternion));
	XMStoreFloat3(&m_view, XMVector3Rotate(XMLoadFloat3(&m_view), quaternion));
	XMStoreFloat3(&m_up, XMVector3Rotate(XMLoadFloat3(&m_up), quaternion));
}

void TargetCamera::rotateAroundVertically(float angle) {
	XMVECTOR vTarget = XMLoadFloat3(&m_target) - XMLoadFloat3(&m_position);
	XMVECTOR vUp = XMLoadFloat3(&m_up);
	XMVector3ComponentsFromNormal(&XMVECTOR(), &vUp, XMLoadFloat3(&m_up), XMVector3Normalize(vTarget));
	XMVECTOR quaternion = XMQuaternionRotationNormal(XMVector3Normalize(vUp), angle);
	XMStoreFloat3(&m_position, XMLoadFloat3(&m_target) + XMVector3Rotate(-vTarget, quaternion));
	XMStoreFloat3(&m_view, XMVector3Rotate(XMLoadFloat3(&m_view), quaternion));
	XMStoreFloat3(&m_up, XMVector3Rotate(XMLoadFloat3(&m_up), quaternion));
}

void TargetCamera::spin(float angle) {
	XMVECTOR quaternion = XMQuaternionRotationNormal(XMVector3Normalize(
		XMLoadFloat3(&m_target) - XMLoadFloat3(&m_position)), angle);
	XMStoreFloat3(&m_view, XMVector3Rotate(XMLoadFloat3(&m_view), quaternion));
	XMStoreFloat3(&m_up, XMVector3Rotate(XMLoadFloat3(&m_up), quaternion));
}

void TargetCamera::tilt(float angle) {
	XMVECTOR vTarget = XMLoadFloat3(&m_target) - XMLoadFloat3(&m_position);
	XMVECTOR quaternion = XMQuaternionRotationAxis(getRight(), angle);
	XMVECTOR vNewView = XMVector3Rotate(XMLoadFloat3(&m_view), quaternion);
	if (XMVectorGetX(XMVector3AngleBetweenVectors(vNewView, vTarget)) > MAX_TILT)
		return;
	XMVECTOR vNewUp = XMVector3Rotate(XMLoadFloat3(&m_up), quaternion);
	if (XMVectorGetX(XMVector3AngleBetweenVectors(vNewUp, vTarget)) < MAX_TILT)
		return;
	XMStoreFloat3(&m_view, vNewView);
	XMStoreFloat3(&m_up, vNewUp);
}

DirectX::XMFLOAT4X4 TargetCamera::calculateViewProjMatrix(float aspectWdivH) {
	XMFLOAT4X4 result;
	float maxDist = XMVectorGetX(XMVector3Length(XMLoadFloat3(&m_position) - XMLoadFloat3(&m_target)));
	maxDist += maxDist > m_radius ? m_radius : - m_radius;
	XMStoreFloat4x4(&result, XMMatrixTranspose(XMMatrixLookToLH(XMLoadFloat3(&m_position), XMLoadFloat3(&m_view), XMLoadFloat3(&m_up))*
											   XMMatrixPerspectiveFovLH(m_fov, aspectWdivH, maxDist / 100000.f, maxDist)));
	return result;
}
