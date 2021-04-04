#pragma once

namespace Utils {

	extern const float Epsilon;

	bool solveTriangleBySSA(float* a, float b, float c, float beta, bool takeObtuseAngle = false);

	float solveTriangleBySAS(float b, float c, float alpha);
}
