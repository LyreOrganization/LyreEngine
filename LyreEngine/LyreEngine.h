#pragma once

class Planet;
class Camera;

namespace LyreEngine
{
	HRESULT initWindow(HINSTANCE hInst, int nCmdShow, WNDPROC WndProc);
	ID3D11Device* getDevice();
	ID3D11DeviceContext* getContext();
	ID3D11Buffer* getCameraCB();
	ID3D11Buffer* getViewProjCB();
	ID3D11Buffer* getLightingCB();
	ID3D11Buffer* getLodCB();
	ID3D11SamplerState* getSampler2D();
	Camera* getCamera();
	void getClientWH(UINT & width, UINT & height);
	HRESULT readShaderFromFile(const WCHAR* szFileName, std::vector<char> &shaderBytecode);
	void render(DWORD ticksPerFrame);
};