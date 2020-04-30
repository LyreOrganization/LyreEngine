#include <Windows.h>
#include "WinApp.h"

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
	WinApp* app = new WinApp(hInstance, hPrevInstance, lpCmdLine, nCmdShow);
	app->Run();
	delete app;

	return S_OK;
}