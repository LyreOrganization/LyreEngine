#include "stdafx.h"

#include "LyreEngine.h"
#include "Camera.h"
#include "Keyboard.h"

#pragma enable_d3d11_debug_symbols

using namespace std;
using namespace DirectX;

void setupKeyboardLayout();

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
		//if (wParam == VK_ESCAPE)
		//	exit(0);
		//else
			Keyboard::press(wParam);
		break;
	case WM_KEYUP:
		Keyboard::release(wParam);
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

	setupKeyboardLayout();
	Keyboard::onTriggered(Action::Exit, []() {
		exit(0);
	});

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

			Keyboard::process();
			LyreEngine::render();
		}
	}
	return (int)msg.wParam;
}

void setupKeyboardLayout() {
	KeyLayout keys;

	/*keys[VK_UP] = Action::Camera_MoveForward;
	keys[VK_DOWN] = Action::Camera_MoveBackward;
	keys[VK_RIGHT] = Action::Camera_MoveRight;
	keys[VK_LEFT] = Action::Camera_MoveLeft;*/

	keys[WindowsLetterIdx('W')] = Action::Camera_MoveForward;
	keys[WindowsLetterIdx('A')] = Action::Camera_MoveLeft;
	keys[WindowsLetterIdx('S')] = Action::Camera_MoveBackward;
	keys[WindowsLetterIdx('D')] = Action::Camera_MoveRight;

	keys[VK_ESCAPE] = Action::Exit;

	Keyboard::setLayout(keys);
}
