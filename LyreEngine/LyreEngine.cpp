#include "stdafx.h"

#include "LyreEngine.h"

#include "resource.h"

#include "Planet.h"
#include "Camera.h"
#include "Keyboard.h"

using namespace std;
using namespace DirectX;

HINSTANCE							LyreEngine::s_hInstance = nullptr;
HWND								LyreEngine::s_hWindow = nullptr;

D3D_DRIVER_TYPE						LyreEngine::s_driverType = D3D_DRIVER_TYPE_NULL;
D3D_FEATURE_LEVEL					LyreEngine::s_featureLevel = D3D_FEATURE_LEVEL_11_0;

CComPtr<ID3D11Device>				LyreEngine::s_iDevice = nullptr;
CComPtr<ID3D11DeviceContext>		LyreEngine::s_iContext = nullptr;

CComPtr<IDXGISwapChain>				LyreEngine::s_iSwapChain = nullptr;

CComPtr<ID3D11RenderTargetView>		LyreEngine::s_iRTV = nullptr;

CComPtr<ID3D11DepthStencilState>	LyreEngine::s_iDSState = nullptr;
CComPtr<ID3D11DepthStencilView>		LyreEngine::s_iDSV = nullptr;

CComPtr<ID3D11BlendState>			LyreEngine::s_iBlendState = nullptr;

CComPtr<ID3D11RasterizerState>		LyreEngine::s_iRasterizerStateWireframe = nullptr;
CComPtr<ID3D11RasterizerState>		LyreEngine::s_iRasterizerStateSolid = nullptr;

CComPtr<ID3D11Buffer>				LyreEngine::s_iViewProjConstantBuffer = nullptr;

unique_ptr<Planet>					LyreEngine::s_pPlanet;
unique_ptr<Camera>					LyreEngine::s_pCamera;

void LyreEngine::render()
{
	static DWORD s_previousTime = GetTickCount();
	DWORD tpf = (GetTickCount() - s_previousTime);
	s_previousTime = GetTickCount();

	float clearColor[4] = { 0.2f, 0.3f, 0.5f, 1.0f };
	s_iContext->ClearRenderTargetView(s_iRTV, clearColor);
	s_iContext->ClearDepthStencilView(s_iDSV, D3D11_CLEAR_DEPTH, 1.0f, 0);

	s_iContext->RSSetState(s_iRasterizerStateWireframe);

	ViewProjConstantBuffer vpcb;
	vpcb.viewProj = s_pCamera->getViewProj(WND_WIDTH / (FLOAT)WND_HEIGHT);
	s_iContext->UpdateSubresource(s_iViewProjConstantBuffer, 0, nullptr, &vpcb, 0, 0);

	s_pPlanet->render();

	s_iSwapChain->Present(0, 0);
}

void LyreEngine::GetClientWH(UINT &width, UINT &height)
{
	RECT rc;
	GetClientRect(s_hWindow, &rc);
	width = rc.right - rc.left;
	height = rc.bottom - rc.top;
}

HRESULT LyreEngine::initWindow(HINSTANCE hInst, int nCmdShow, WNDPROC WndProc)
{
	WNDCLASSEX wcex;
	{
		wcex.cbSize = sizeof(WNDCLASSEX);
		wcex.style = CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc = WndProc;
		wcex.cbClsExtra = 0;
		wcex.cbWndExtra = 0;
		wcex.hInstance = hInst;
		wcex.hIcon = LoadIcon(hInst, (LPCTSTR)IDI_ICON);
		wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
		wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
		wcex.lpszMenuName = nullptr;
		wcex.lpszClassName = L"LyreEngine";
		wcex.hIconSm = LoadIcon(wcex.hInstance, (LPCTSTR)IDI_ICON);
	}
	if (!RegisterClassEx(&wcex))
		return E_FAIL;
	s_hInstance = hInst;
	RECT rc = { 0, 0, WND_WIDTH, WND_HEIGHT };
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
	s_hWindow = CreateWindow(L"LyreEngine", L"LyreEngine", WS_OVERLAPPEDWINDOW, WND_POS_X,
		WND_POS_Y, rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, hInst, nullptr);
	if (!s_hWindow)
		return E_FAIL;
	ShowCursor(FALSE);
	SetCursorPos(WND_WIDTH / 2, WND_HEIGHT / 2);
	ShowWindow(s_hWindow, nCmdShow);

	return S_OK;
}

HRESULT LyreEngine::init()
{
	HRESULT hr;

	UINT width, height;
	GetClientWH(width, height);

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
		swapChainDesc.OutputWindow = s_hWindow;
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.SampleDesc.Quality = 0;
		swapChainDesc.Windowed = TRUE;
	}
	for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
	{
		s_driverType = driverTypes[driverTypeIndex];
		hr = D3D11CreateDeviceAndSwapChain(nullptr, s_driverType, nullptr, 0, featureLevels, numFeatureLevels,
			D3D11_SDK_VERSION, &swapChainDesc, &s_iSwapChain, &s_iDevice, &s_featureLevel, &s_iContext);
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
		vp.Width = (FLOAT)width;
		vp.Height = (FLOAT)height;
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

	//Planet
	s_pPlanet = make_unique<Planet>();
	hr = s_pPlanet->init();
	if (FAILED(hr))
		throw runtime_error("Planet init failed!");

	//Camera
	s_pCamera = make_unique<Camera>();

	Keyboard::on(Action::Camera_MoveForward, [](DWORD tpf) {
		s_pCamera->moveForward(0.001f*tpf);
	});
	Keyboard::on(Action::Camera_MoveBackward, [](DWORD tpf) {
		s_pCamera->moveBackward(0.001f*tpf);
	});
	Keyboard::on(Action::Camera_MoveRight, [](DWORD tpf) {
		s_pCamera->moveRight(0.001f*tpf);
	});
	Keyboard::on(Action::Camera_MoveLeft, [](DWORD tpf) {
		s_pCamera->moveLeft(0.001f*tpf);
	});

	return S_OK;
}

ID3D11Device* LyreEngine::getDevice()
{
	if (s_iDevice == nullptr)
		if ( FAILED(init()) ) {
			throw runtime_error("D3D11 init failed!");
		}

	return s_iDevice;
}

ID3D11DeviceContext* LyreEngine::getContext()
{
	if (s_iContext == nullptr)
		if ( FAILED(init()) ) {
			throw runtime_error("D3D11 init failed!");
		}

	return s_iContext;
}

ID3D11Buffer* LyreEngine::getViewProj()
{
	return s_iViewProjConstantBuffer;
}

Camera * LyreEngine::getCamera()
{
	return s_pCamera.get();
}

HRESULT LyreEngine::ReadShaderFromFile(WCHAR* szFileName, std::vector<char> &shaderBytecode)
{
	std::ifstream input(szFileName, std::ios::binary);
	shaderBytecode = std::vector<char>(std::istreambuf_iterator<char>(input),
		std::istreambuf_iterator<char>());
	return (shaderBytecode.size() == 0 ? E_FAIL : S_OK);
}
