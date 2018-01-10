#include "stdafx.h"

#include "TargetCamera.h"

TargetCamera::TargetCamera(float x, float y, float z, float targ_x, float targ_y, float targ_z)
	: m_target { targ_x, targ_y, targ_z }, m_headingAngle(0.f)
{}

void TargetCamera::Look(float targ_x, float targ_y, float targ_z) {
	m_target = { targ_x,targ_y,targ_z };
}

void TargetCamera::GoToTarget(float distance) {
	XMStoreFloat3(&m_position, XMLoadFloat3(&m_position) +
				  XMVector3Normalize(XMLoadFloat3(&m_target)
									 - XMLoadFloat3(&m_position)) * distance);
}

void TargetCamera::ApproachToTarget(float percentage) {
	//SAME AS : position += (target - position) * p
	XMVECTOR direction = XMLoadFloat3(&m_target) - XMLoadFloat3(&m_position);
	float moveLength = (XMVectorGetX(XMVector3Length(direction)) - PLANET_RAD) * percentage / 100.f;
	XMStoreFloat3(&m_position, XMLoadFloat3(&m_position) +
		(moveLength > 0 ?
		 XMVector3ClampLength(direction, moveLength, moveLength) :
		 -XMVector3ClampLength(direction, -moveLength, -moveLength)));
}

void TargetCamera::RotateAroundTargetH(float angle) {
	XMStoreFloat3(&m_position, XMLoadFloat3(&m_target) +
				  XMVector3Rotate(XMLoadFloat3(&m_position) - XMLoadFloat3(&m_target),
								  XMQuaternionRotationNormal(XMLoadFloat3(&m_upAxis), angle)));
}

void TargetCamera::RotateAroundTargetV(float angle) {
	XMVECTOR direction = XMLoadFloat3(&m_position) - XMLoadFloat3(&m_target);
	XMVECTOR right_axis = XMVector3Cross(direction, XMLoadFloat3(&m_upAxis));
	direction = XMVector3Rotate(direction, XMQuaternionRotationAxis(right_axis, angle));
	XMStoreFloat3(&m_position, XMLoadFloat3(&m_target) + direction);
	XMStoreFloat3(&m_upAxis, XMVector3Normalize(XMVector3Cross(right_axis, direction)));
}

void TargetCamera::RotateUpAxis(float angle) {
	XMStoreFloat3(&m_upAxis, XMVector3Rotate(XMLoadFloat3(&m_upAxis),
											 XMQuaternionRotationAxis(XMLoadFloat3(&m_target) - XMLoadFloat3(&m_position), angle)));
}

void TargetCamera::RotateHeading(float angle) {
	m_headingAngle += angle;
}

XMFLOAT3 TargetCamera::at() {
	XMFLOAT3 result;
	XMVECTOR direction = XMLoadFloat3(&m_target) - XMLoadFloat3(&m_position);
	XMVECTOR right_axis = XMVector3Cross(direction, XMLoadFloat3(&m_upAxis));
	XMVECTOR at = XMVector3Rotate(direction, XMQuaternionRotationAxis(right_axis, m_headingAngle));
	at = XMLoadFloat3(&m_position) + XMVector3Normalize(at);
	XMStoreFloat3(&result, at);
	return result;
}

XMFLOAT3 TargetCamera::up() {
	XMFLOAT3 result;
	XMVECTOR direction = XMLoadFloat3(&m_target) - XMLoadFloat3(&m_position);
	XMVECTOR right_axis = XMVector3Cross(direction, XMLoadFloat3(&m_upAxis));
	XMStoreFloat3(&result, XMVector3Rotate(XMLoadFloat3(&m_upAxis),
										   XMQuaternionRotationAxis(right_axis, m_headingAngle)));
	return result;
}

float TargetCamera::TargetDist() {
	float len;
	XMStoreFloat(&len, XMVector3Length(XMLoadFloat3(&m_target) - XMLoadFloat3(&m_position)));
	return len;
}
