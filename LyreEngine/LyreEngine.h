#pragma once

class Planet;
class FreeCamera;

namespace LyreEngine
{
	HRESULT initWindow(HINSTANCE hInst, int nCmdShow, WNDPROC WndProc);
	ID3D11Device* getDevice();
	ID3D11DeviceContext* getContext();
	ID3D11Buffer* getViewProj();
	ID3D11Buffer* getLighting();
	FreeCamera* getCamera();
	void getClientWH(UINT & width, UINT & height);
	HRESULT readShaderFromFile(WCHAR* szFileName, std::vector<char> &shaderBytecode);
	void render(DWORD ticksPerFrame);
};