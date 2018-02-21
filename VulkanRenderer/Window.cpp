#include "Window.h"
#include <stdlib.h>  
#include <string.h>  
#include <tchar.h>  
#include <iostream>
#include "vulkanRender/util.h"

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;

	Window * window = reinterpret_cast<Window*>(
		GetWindowLongPtrW(hWnd, GWLP_USERDATA));

	switch (message)
	{
	case WM_CLOSE:
		window->Close();
		return 0;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		window->Close();
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
		break;
	}

	return 0;
}

uint64_t	Window::_win32_class_id_counter = 0;

Window::Window(uint32_t size_x, uint32_t size_y, std::string title)
{
	HINSTANCE hInstance = GetModuleHandle(nullptr);
	WNDCLASSEX wcex;

	_win32_class_name = title + "_" + std::to_string(_win32_class_id_counter);
	_win32_class_id_counter++;

	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APPLICATION));
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = _win32_class_name.c_str();
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_APPLICATION));

	if (!RegisterClassEx(&wcex))
	{
		MessageBox(NULL,
			_T("Call to RegisterClassEx failed!"),
			_T("Win32 Guided Tour"),
			NULL);
	}
	hWnd = CreateWindow(
		_win32_class_name.c_str(),
		title.c_str(),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		size_x, size_y,
		NULL,
		NULL,
		hInstance,
		NULL
	);
	SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)this);
	if (!hWnd)
	{
		MessageBox(NULL,
			_T("Call to CreateWindow failed!"),
			_T("Win32 Guided Tour"),
			NULL);
	}

	// The parameters to ShowWindow explained:  
	// hWnd: the value returned from CreateWindow  
	// nCmdShow: the fourth parameter from WinMain  
	ShowWindow(hWnd, SW_SHOW);
	UpdateWindow(hWnd);
}


Window::~Window()
{
}

bool Window::Update()
{
	MSG msg;
	if (PeekMessage(&msg, hWnd, 0, 0, PM_REMOVE)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return bisOpen;
}

vk::SurfaceKHR Window::createSurface(vk::Instance inst)
{
	vk::SurfaceKHR surface = nullptr;
	auto const createInfo = vk::Win32SurfaceCreateInfoKHR().setHinstance(hInstance).setHwnd(hWnd);
	VK_CHECK_RESULT(inst.createWin32SurfaceKHR(&createInfo, NULL, &surface));
	return surface;
}
