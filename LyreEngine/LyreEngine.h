#pragma once

#include "Camera.h"

namespace LyreEngine
{
	HRESULT initWindow(HINSTANCE hInst, int nCmdShow, WNDPROC WndProc);
	ID3D11Device* getDevice();
	ID3D11DeviceContext* getContext();
	ID3D11Buffer* getViewCB();
	ID3D11Buffer* getProjectionCB();
	ID3D11Buffer* getLightingCB();
	DirectX::XMFLOAT3 getLightingDirection();
	ID3D11Buffer* getLodCB();
	ID3D11SamplerState* getSampler2D();
	Camera* getCamera();
	void getClientWH(UINT & width, UINT & height);
	HRESULT readShaderFromFile(const WCHAR* szFileName, std::vector<char> &shaderBytecode);
	void render(DWORD ticksPerFrame);
};