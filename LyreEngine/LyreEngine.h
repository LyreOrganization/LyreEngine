#pragma once

class Planet;
class Camera;

namespace LyreEngine
{
	struct ViewProjConstantBuffer {
		DirectX::XMFLOAT4X4 viewProj;
	};

	HRESULT initWindow(HINSTANCE hInst, int nCmdShow, WNDPROC WndProc);
	ID3D11Device* getDevice();
	ID3D11DeviceContext* getContext();
	ID3D11Buffer* getViewProj();
	Camera* getCamera();
	void GetClientWH(UINT & width, UINT & height);
	HRESULT ReadShaderFromFile(WCHAR* szFileName, std::vector<char> &shaderBytecode);
	void render();
};