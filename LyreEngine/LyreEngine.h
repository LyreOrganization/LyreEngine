#pragma once

#include "Camera.h"

namespace LyreEngine
{
	HRESULT initWindow(HINSTANCE hInst, int nCmdShow, WNDPROC WndProc);
	ID3D11Device* getDevice();
	ID3D11DeviceContext* getContext();
	ID3D11Buffer* getCameraCB();
	ID3D11Buffer* getViewProjCB();
	ID3D11Buffer* getLightingCB();
	DirectX::XMFLOAT3 getLightingDirection();
	ID3D11Buffer* getLodCB();
	ID3D11SamplerState* getSamplerLinear();
	ID3D11SamplerState* getSamplerPoint();
	Camera* getCamera();
	void getClientWH(UINT & width, UINT & height);
	HRESULT readShaderFromFile(const WCHAR* szFileName, std::vector<char> &shaderBytecode);
	void render(DWORD ticksPerFrame);
};