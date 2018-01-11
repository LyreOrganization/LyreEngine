#include "stdafx.h"

#include "LyreEngine.h"

#include "resource.h"

#include "Planet.h"
#include "FreeCamera.h"
#include "Keyboard.h"
#include "Actions.h"

using namespace std;
using namespace DirectX;

namespace {

	struct ViewProjConstantBuffer {
		XMFLOAT4X4 viewProj;
	};

	struct LightingConstantBuffer {
		XMFLOAT3 direction;
		float power;
		XMFLOAT4 diffuse;
	};

	HINSTANCE							g_hInstance = nullptr;
	HWND								g_hWindow = nullptr;

	D3D_DRIVER_TYPE						s_driverType = D3D_DRIVER_TYPE_NULL;
	D3D_FEATURE_LEVEL					s_featureLevel = D3D_FEATURE_LEVEL_11_0;

	CComPtr<ID3D11Device>				s_iDevice = nullptr;
	CComPtr<ID3D11DeviceContext>		s_iContext = nullptr;

	CComPtr<IDXGISwapChain>				s_iSwapChain = nullptr;

	CComPtr<ID3D11RenderTargetView>		s_iRTV = nullptr;

	CComPtr<ID3D11DepthStencilState>	s_iDSState = nullptr;
	CComPtr<ID3D11DepthStencilView>		s_iDSV = nullptr;

	CComPtr<ID3D11BlendState>			s_iBlendState = nullptr;

	CComPtr<ID3D11RasterizerState>		s_iRasterizerStateWireframe = nullptr;
	CComPtr<ID3D11RasterizerState>		s_iRasterizerStateSolid = nullptr;

	CComPtr<ID3D11Buffer>				s_iViewProjConstantBuffer = nullptr;
	CComPtr<ID3D11Buffer>				s_iLightingConstantBuffer = nullptr;

	std::unique_ptr<Planet>				s_pPlanet;

	std::unique_ptr<FreeCamera>				s_pCamera;

	HRESULT init() {
		HRESULT hr;

		UINT width, height;
		LyreEngine::getClientWH(width, height);

		D3D_DRIVER_TYPE driverTypes[] =
		{
			D3D_DRIVER_TYPE_HARDWARE,
			D3D_DRIVER_TYPE_WARP,
			D3D_DRIVER_TYPE_REFERENCE
		};
		UINT numDriverTypes = ARRAYSIZE(driverTypes);
		D3D_FEATURE_LEVEL featureLevels[] =
		{
			D3D_FEATURE_LEVEL_11_0
		};
		UINT numFeatureLevels = ARRAYSIZE(featureLevels);
		DXGI_SWAP_CHAIN_DESC swapChainDesc;
		{
			ZeroStruct(swapChainDesc);
			swapChainDesc.BufferCount = 1;
			swapChainDesc.BufferDesc.Width = width;
			swapChainDesc.BufferDesc.Height = height;
			swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
			swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
			swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
			swapChainDesc.OutputWindow = g_hWindow;
			swapChainDesc.SampleDesc.Count = 1;
			swapChainDesc.SampleDesc.Quality = 0;
			swapChainDesc.Windowed = TRUE;
		}
		for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++) {
			s_driverType = driverTypes[driverTypeIndex];
			hr = D3D11CreateDeviceAndSwapChain(nullptr, s_driverType, nullptr, 0, featureLevels,
											   numFeatureLevels, D3D11_SDK_VERSION, &swapChainDesc,
											   &s_iSwapChain, &s_iDevice, &s_featureLevel, &s_iContext);
			if (SUCCEEDED(hr))
				break;
		}
		if (FAILED(hr))
			return hr;

		//Creating RTV
		CComPtr<ID3D11Texture2D> pSwapChainBuffer = nullptr;
		hr = s_iSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pSwapChainBuffer);
		if (FAILED(hr))
			return hr;
		hr = s_iDevice->CreateRenderTargetView(pSwapChainBuffer, nullptr, &s_iRTV);
		if (FAILED(hr))
			return hr;

		//Creating DSBuffer
		CComPtr<ID3D11Texture2D> pDSBuffer = nullptr;
		D3D11_TEXTURE2D_DESC depthSurface;
		{
			ZeroStruct(depthSurface);
			depthSurface.Width = width;
			depthSurface.Height = height;
			depthSurface.MipLevels = 1;
			depthSurface.ArraySize = 1;
			depthSurface.Format = DXGI_FORMAT_D32_FLOAT;
			depthSurface.SampleDesc.Count = 1;
			depthSurface.SampleDesc.Quality = 0;
			depthSurface.Usage = D3D11_USAGE_DEFAULT;
			depthSurface.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		}
		hr = s_iDevice->CreateTexture2D(&depthSurface, nullptr, &pDSBuffer);
		if (FAILED(hr))
			return hr;
		//Creating DSState
		D3D11_DEPTH_STENCIL_DESC depthStencil;
		{
			ZeroStruct(depthStencil);
			depthStencil.DepthEnable = true;
			depthStencil.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
			depthStencil.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
			depthStencil.StencilEnable = true;
			depthStencil.StencilReadMask = 0xFF;
			depthStencil.StencilWriteMask = 0xFF;
			depthStencil.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
			depthStencil.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
			depthStencil.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
			depthStencil.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
			depthStencil.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
			depthStencil.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
			depthStencil.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
			depthStencil.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
		}
		hr = s_iDevice->CreateDepthStencilState(&depthStencil, &s_iDSState);
		if (FAILED(hr))
			return hr;
		s_iContext->OMSetDepthStencilState(s_iDSState, 1);
		//Creting DSV
		D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
		{
			ZeroStruct(depthStencilViewDesc);
			depthStencilViewDesc.Format = depthSurface.Format;
			depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
			depthStencilViewDesc.Texture2D.MipSlice = 0;
		}
		hr = s_iDevice->CreateDepthStencilView(pDSBuffer, &depthStencilViewDesc, &s_iDSV);
		if (FAILED(hr))
			return hr;
		s_iContext->OMSetRenderTargets(1, &s_iRTV.p, s_iDSV);

		D3D11_BLEND_DESC blendStateDesc;
		{
			ZeroStruct(blendStateDesc);
			blendStateDesc.AlphaToCoverageEnable = 0;
			blendStateDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
		}
		hr = s_iDevice->CreateBlendState(&blendStateDesc, &s_iBlendState);
		if (FAILED(hr))
			return hr;
		float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
		s_iContext->OMSetBlendState(s_iBlendState, blendFactor, 0xffffffff);

		//Setting rasterizer states
		///solid
		D3D11_RASTERIZER_DESC rasterizerState;
		{
			ZeroStruct(rasterizerState);
			rasterizerState.FillMode = D3D11_FILL_SOLID;
			rasterizerState.CullMode = D3D11_CULL_BACK;
			rasterizerState.FrontCounterClockwise = false;
			rasterizerState.DepthBias = 0;
			rasterizerState.SlopeScaledDepthBias = 0.0f;
			rasterizerState.DepthBiasClamp = 0.0f;
			rasterizerState.DepthClipEnable = true;
			rasterizerState.ScissorEnable = false;
			rasterizerState.MultisampleEnable = false;
			rasterizerState.AntialiasedLineEnable = false;
		}
		hr = s_iDevice->CreateRasterizerState(&rasterizerState, &s_iRasterizerStateSolid);
		if (FAILED(hr))
			return hr;
		///wireframe
		{
			ZeroStruct(rasterizerState);
			rasterizerState.FillMode = D3D11_FILL_WIREFRAME;
			rasterizerState.CullMode = D3D11_CULL_BACK;
			rasterizerState.FrontCounterClockwise = false;
			rasterizerState.DepthBias = 0;
			rasterizerState.SlopeScaledDepthBias = 0.0f;
			rasterizerState.DepthBiasClamp = 0.0f;
			rasterizerState.DepthClipEnable = true;
			rasterizerState.ScissorEnable = false;
			rasterizerState.MultisampleEnable = false;
			rasterizerState.AntialiasedLineEnable = false;
		}
		hr = s_iDevice->CreateRasterizerState(&rasterizerState, &s_iRasterizerStateWireframe);
		if (FAILED(hr))
			return hr;
		//Setup the viewport
		D3D11_VIEWPORT vp;
		{
			vp.Width = static_cast<FLOAT>(width);
			vp.Height = static_cast<FLOAT>(height);
			vp.MinDepth = 0.0f;
			vp.MaxDepth = 1.0f;
			vp.TopLeftX = 0;
			vp.TopLeftY = 0;
		}
		s_iContext->RSSetViewports(1, &vp);

		D3D11_BUFFER_DESC bufferDesc;

		//ViewProj constant buffer
		{
			ZeroStruct(bufferDesc);
			bufferDesc.Usage = D3D11_USAGE_DEFAULT;
			bufferDesc.ByteWidth = sizeof(ViewProjConstantBuffer);
			bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		}
		hr = s_iDevice->CreateBuffer(&bufferDesc, nullptr, &s_iViewProjConstantBuffer);
		if (FAILED(hr))
			return hr;

		//Lighting constant buffer
		{
			ZeroStruct(bufferDesc);
			bufferDesc.Usage = D3D11_USAGE_DEFAULT;
			bufferDesc.ByteWidth = sizeof(LightingConstantBuffer);
			bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		}
		hr = s_iDevice->CreateBuffer(&bufferDesc, nullptr, &s_iLightingConstantBuffer);
		if (FAILED(hr))
			return hr;

		//Planet
		s_pPlanet = make_unique<Planet>();
		hr = s_pPlanet->init();
		if (FAILED(hr))
			throw runtime_error("Planet init failed!");

		//Camera
		s_pCamera = make_unique<FreeCamera>();
		/*Setup camera actions*/ {
			Keyboard::on(Action::Camera_RollCW, [](DWORD ticksPerFrame) {
				s_pCamera->roll(-0.001f*ticksPerFrame);
			});
			Keyboard::on(Action::Camera_RollCCW, [](DWORD ticksPerFrame) {
				s_pCamera->roll(0.001f*ticksPerFrame);
			});
			Keyboard::on(Action::Camera_MoveForward, [](DWORD ticksPerFrame) {
				s_pCamera->moveForward(0.001f*ticksPerFrame);
			});
			Keyboard::on(Action::Camera_MoveBackward, [](DWORD ticksPerFrame) {
				s_pCamera->moveBackward(0.001f*ticksPerFrame);
			});
			Keyboard::on(Action::Camera_MoveRight, [](DWORD ticksPerFrame) {
				s_pCamera->moveRight(0.001f*ticksPerFrame);
			});
			Keyboard::on(Action::Camera_MoveLeft, [](DWORD ticksPerFrame) {
				s_pCamera->moveLeft(0.001f*ticksPerFrame);
			});
		}

		return S_OK;
	}

}

void LyreEngine::render(DWORD ticksPerFrame) {
	float clearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	s_iContext->ClearRenderTargetView(s_iRTV, clearColor);
	s_iContext->ClearDepthStencilView(s_iDSV, D3D11_CLEAR_DEPTH, 1.0f, 0);

	s_iContext->RSSetState(s_iRasterizerStateSolid);

	ViewProjConstantBuffer vpcb;
	vpcb.viewProj = s_pCamera->getViewProj(WND_WIDTH / static_cast<FLOAT>(WND_HEIGHT));
	s_iContext->UpdateSubresource(s_iViewProjConstantBuffer, 0, nullptr, &vpcb, 0, 0);

	LightingConstantBuffer lcb;
	lcb.diffuse = { 1.0f, 0.9f, 0.7f, 1.f };
	static float s_angle = 0;
	s_angle += ticksPerFrame / 1000.f;
	lcb.direction = { sin(s_angle), 0.f, cos(s_angle) };
	lcb.power = 0.95f;
	s_iContext->UpdateSubresource(s_iLightingConstantBuffer, 0, nullptr, &lcb, 0, 0);

	s_pPlanet->render();

	s_iSwapChain->Present(0, 0);
}

void LyreEngine::getClientWH(UINT &width, UINT &height) {
	RECT rc;
	GetClientRect(g_hWindow, &rc);
	width = rc.right - rc.left;
	height = rc.bottom - rc.top;
}

HRESULT LyreEngine::initWindow(HINSTANCE hInst, int nCmdShow, WNDPROC WndProc) {
	WNDCLASSEX wcex;
	{
		wcex.cbSize = sizeof(WNDCLASSEX);
		wcex.style = CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc = WndProc;
		wcex.cbClsExtra = 0;
		wcex.cbWndExtra = 0;
		wcex.hInstance = hInst;
		wcex.hIcon = LoadIcon(hInst, reinterpret_cast<LPCTSTR>(IDI_ICON));
		wcex.hCursor = LoadCursor(nullptr, reinterpret_cast<LPCTSTR>IDC_ARROW);
		wcex.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
		wcex.lpszMenuName = nullptr;
		wcex.lpszClassName = L"LyreEngine";
		wcex.hIconSm = LoadIcon(wcex.hInstance, reinterpret_cast<LPCTSTR>(IDI_ICON));
	}
	if (!RegisterClassEx(&wcex))
		return E_FAIL;
	g_hInstance = hInst;
	RECT rc = { 0, 0, WND_WIDTH, WND_HEIGHT };
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
	g_hWindow = CreateWindow(L"LyreEngine", L"LyreEngine", WS_OVERLAPPEDWINDOW, WND_POS_X,
							 WND_POS_Y, rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, hInst, nullptr);
	if (!g_hWindow)
		return E_FAIL;
	ShowCursor(FALSE);
	SetCursorPos(WND_WIDTH / 2, WND_HEIGHT / 2);
	ShowWindow(g_hWindow, nCmdShow);

	return S_OK;
}

ID3D11Device* LyreEngine::getDevice() {
	if (s_iDevice == nullptr)
		if (FAILED(init())) {
			throw runtime_error("D3D11 init failed!");
		}

	return s_iDevice;
}

ID3D11DeviceContext* LyreEngine::getContext() {
	if (s_iContext == nullptr)
		if (FAILED(init())) {
			throw runtime_error("D3D11 init failed!");
		}

	return s_iContext;
}

ID3D11Buffer* LyreEngine::getViewProj() {
	return s_iViewProjConstantBuffer;
}

ID3D11Buffer* LyreEngine::getLighting() {
	return s_iLightingConstantBuffer;
}

FreeCamera * LyreEngine::getCamera() {
	return s_pCamera.get();
}

HRESULT LyreEngine::readShaderFromFile(WCHAR* szFileName, std::vector<char> &shaderBytecode) {
	std::ifstream input(szFileName, std::ios::binary);
	shaderBytecode = std::vector<char>(std::istreambuf_iterator<char>(input),
									   std::istreambuf_iterator<char>());
	return (shaderBytecode.size() == 0 ? E_FAIL : S_OK);
}
