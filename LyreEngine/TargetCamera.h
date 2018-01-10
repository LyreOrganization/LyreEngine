#pragma once

class TargetCamera final {
public:
	TargetCamera(float x, float y, float z,
				 float targ_x, float targ_y, float targ_z);
	void Look(float targ_x, float targ_y, float targ_z);
	void GoToTarget(float distance);
	void ApproachToTarget(float percentage);
	void RotateAroundTargetH(float angle);
	void RotateAroundTargetV(float angle);
	virtual void RotateUpAxis(float angle);
	virtual void RotateHeading(float angle);
	virtual DirectX::XMFLOAT3 up();
	virtual DirectX::XMFLOAT3 at();
	float TargetDist();

protected:
	DirectX::XMFLOAT3 m_target;
	float m_headingAngle;
};