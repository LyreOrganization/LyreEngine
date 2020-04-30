#pragma once

#include "GeometryDX.h"

class UVSphere : public GeometryDX {

	inline DirectX::XMFLOAT3 toDecart(float, float, float);

public:

	UVSphere(float radius, int stacks, int slices, bool reverseFacing = false);
};
