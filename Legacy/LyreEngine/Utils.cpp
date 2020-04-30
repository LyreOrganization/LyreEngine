#include "stdafx.h"

using namespace DirectX;

const float Utils::Epsilon = 0.001f;

bool Utils::solveTriangleBySSA(float* a, float b, float c, float beta, bool takeObtuseAngle) {
	if (fabsf(beta) < Epsilon) {
		if (b < c) {
			*a = c - b;
			return true;
		}
		return false;
	}
	if (fabsf(beta - XM_PI) < Epsilon) {
		if (b > c) {
			*a = b - c;
			return true;
		}
		return false;
	}

	float sinBeta = sinf(beta);
	float D = c / b * sinBeta;

	if (D > 1.f || (beta >= XM_PIDIV2 && b <= c)) {
		return false;
	}

	if (fabs(D - 1) < Epsilon) {
		*a = c * cosf(beta);
		return true;
	}

	if (D < 1) {
		float gamma = asinf(D);

		if (b < c && (takeObtuseAngle)) {
			gamma = XM_PI - gamma;
		}
		float alpha = XM_PI - beta - gamma;

		*a = b * sinf(alpha) * sinBeta;
		return true;
	}
}

float Utils::solveTriangleBySAS(float b, float c, float alpha) {
	return sqrtf(b*b + c*c - 2*b*c*cosf(alpha));
}
