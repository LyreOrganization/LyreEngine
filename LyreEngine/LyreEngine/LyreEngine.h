#pragma once

#include "stdafx.h"

class Planet;

class LyreEngine {

public:
	struct ViewProjConstantBuffer
	{
		DirectX::XMFLOAT4X4 view;
		DirectX::XMFLOAT4X4 projection;
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


	static HRESULT init();

public:
	static HRESULT initWindow(HINSTANCE hInst, int nCmdShow, WNDPROC WndProc);
	static ID3D11Device* getDevice();
	static ID3D11DeviceContext* getContext();
	static ID3D11Buffer* getViewProj();
	static void GetClientWH(UINT & width, UINT & height);
	static HRESULT ReadShaderFromFile(WCHAR* szFileName, std::vector<char> &shaderBytecode);
	static void render();
};