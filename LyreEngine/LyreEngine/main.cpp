#include "stdafx.h"

#include "LyreEngine.h"
#include "Camera.h"

#pragma enable_d3d11_debug_symbols

using namespace std;
using namespace DirectX;

LRESULT WINAPI MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	case WM_PAINT:
		ValidateRect(hWnd, nullptr);
		return 0;
	case WM_KEYDOWN:
		if (wParam == VK_ESCAPE)
			exit(0);
		else
			LyreEngine::pressButton(wParam);
		break;
	case WM_KEYUP:
		LyreEngine::releaseButton(wParam);
		break;
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}

int WINAPI wWinMain(HINSTANCE m_hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	if (FAILED(LyreEngine::initWindow(m_hInstance, nCmdShow, MsgProc))) {
		MessageBox(0, L"Window init failed!", 0, 0);
		return EXIT_FAILURE;
	}
	try {
		LyreEngine::getDevice();
	}
	catch (std::runtime_error e) {
		wchar_t msg[100];
		size_t msgLen;
		//Convert c-style string to ebuchiy winapishnyy LPCWSTR
		mbstowcs_s(&msgLen, msg, e.what(), strlen(e.what()));

		MessageBox(0, msg, 0, 0);
		return EXIT_FAILURE;
	}

	MSG msg = { 0 };
	while (WM_QUIT != msg.message)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			UINT width, height;
			LyreEngine::GetClientWH(width, height);

			POINT cursor;
			GetCursorPos(&cursor);
			LyreEngine::getCamera()->tilt((cursor.y - height / 2.f) / 1000.f);
			LyreEngine::getCamera()->pan((width / 2.f - cursor.x) / 1000.f);
			SetCursorPos(width / 2., height / 2.);

			LyreEngine::processControls();
			LyreEngine::render();
		}
	}
	return (int)msg.wParam;
}
