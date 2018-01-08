#pragma once

class Planet;
class Camera;

class LyreEngine final
{
public:
	struct ViewProjConstantBuffer
	{
		DirectX::XMFLOAT4X4 viewProj;
	};

private:
	static HINSTANCE							s_hInstance;
	static HWND									s_hWindow;

	static D3D_DRIVER_TYPE						s_driverType;
	static D3D_FEATURE_LEVEL					s_featureLevel;

	static CComPtr<ID3D11Device>				s_iDevice;
	static CComPtr<ID3D11DeviceContext>			s_iContext;

	static CComPtr<IDXGISwapChain>				s_iSwapChain;

	static CComPtr<ID3D11RenderTargetView>		s_iRTV;

	static CComPtr<ID3D11DepthStencilState>	    s_iDSState;
	static CComPtr<ID3D11DepthStencilView>		s_iDSV;

	static CComPtr<ID3D11BlendState>			s_iBlendState;

	static CComPtr<ID3D11RasterizerState>		s_iRasterizerStateWireframe;
	static CComPtr<ID3D11RasterizerState>		s_iRasterizerStateSolid;

	static CComPtr<ID3D11Buffer>				s_iViewProjConstantBuffer;

	static std::unique_ptr<Planet>				s_pPlanet;

	static std::unique_ptr<Camera>				s_pCamera;

	static std::array<bool, 0x100>				s_keys;

	static HRESULT init();

public:
	static HRESULT initWindow(HINSTANCE hInst, int nCmdShow, WNDPROC WndProc);
	static ID3D11Device* getDevice();
	static ID3D11DeviceContext* getContext();
	static ID3D11Buffer* getViewProj();
	static Camera* getCamera();
	static void GetClientWH(UINT & width, UINT & height);
	static HRESULT ReadShaderFromFile(WCHAR* szFileName, std::vector<char> &shaderBytecode);
	static void render();
	static void pressButton(WPARAM button);
	static void releaseButton(WPARAM button);
	static void processControls();
};