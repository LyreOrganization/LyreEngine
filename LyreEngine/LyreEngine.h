#pragma once

class Planet;
class FreeCamera;

namespace LyreEngine
{
	HRESULT initWindow(HINSTANCE hInst, int nCmdShow, WNDPROC WndProc);
	ID3D11Device* getDevice();
	ID3D11DeviceContext* getContext();
	ID3D11Buffer* getViewCB();
	ID3D11Buffer* getProjectionCB();
	ID3D11Buffer* getLightingCB();
	ID3D11Buffer* getLodCB();
	ID3D11SamplerState* getSampler2D();
	FreeCamera* getCamera();
	void getClientWH(UINT & width, UINT & height);
	HRESULT readShaderFromFile(const WCHAR* szFileName, std::vector<char> &shaderBytecode);
	void render(DWORD ticksPerFrame);
};