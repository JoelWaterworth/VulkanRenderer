#include "windowHandler.h"
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include "engine.h"
#include "vulkanRender/util.h"


#if defined(_WIN32)
#include <tchar.h>


LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;

	Engine* engine = reinterpret_cast<Engine*>(
		GetWindowLongPtrW(hWnd, GWLP_USERDATA));

	switch (message)
	{
	case WM_CLOSE:
		engine->window.Close();
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		engine->window.Close();
		break;
	default:
		break;
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}

uint64_t	WindowHandle::_win32_class_id_counter = 0;

WindowHandle::WindowHandle()
{
}

WindowHandle::WindowHandle(uint32_t size_x, uint32_t size_y, std::string title, Engine* engine)
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
	wcex.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = _win32_class_name.c_str();
	wcex.hIconSm = LoadIcon(NULL, IDI_WINLOGO);

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
	SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)engine);
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
	SetForegroundWindow(hWnd);
	UpdateWindow(hWnd);
}

bool WindowHandle::Update()
{
	MSG msg;
	if (PeekMessage(&msg, hWnd, 0, 0, PM_REMOVE)) {
		switch (msg.message)
		{
		case WM_QUIT:
			PostQuitMessage(0);
			Close();
			break;

		case WM_KEYDOWN:
			activeKeys.insert(MapVirtualKey(msg.wParam, MAPVK_VK_TO_CHAR));
			break;

		case WM_KEYUP:
			activeKeys.erase(MapVirtualKey(msg.wParam, MAPVK_VK_TO_CHAR));
			break;
		}
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return bisOpen;
}

VkSurfaceKHR WindowHandle::createSurface(VkInstance inst)
{
	VkSurfaceKHR surface = nullptr;
	VkWin32SurfaceCreateInfoKHR createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	createInfo.hinstance = hInstance;
	createInfo.hwnd = hWnd;
	VK_CHECK_RESULT(vkCreateWin32SurfaceKHR(inst, &createInfo, NULL, &surface));
	return surface;
}


#else

WindowHandle::WindowHandle(uint32_t size_x, uint32_t size_y, std::string title, Engine* engine) {
    Visual                  *visual;
    int                     depth;
    int                     text_x;
    int                     text_y;
    XSetWindowAttributes    frame_attributes;
    XFontStruct             *fontinfo;
    XGCValues               gr_values;
    GC                      graphical_context;
    char                    hello_string[] = "Hello World";
    int                     hello_string_length = strlen(hello_string);
    _display = XOpenDisplay(NULL);
    visual = DefaultVisual(_display, 0);
    depth  = DefaultDepth(_display, 0);
    frame_attributes.background_pixel = XWhitePixel(_display, 0);
    _Window = XCreateWindow(
        _display,
        XRootWindow(_display, 0),
        0, 0, 400, 400, 5,
        depth, InputOutput, visual, CWBackPixel,&frame_attributes);
    XStoreName(_display, _Window, "Hello World Example");
    XSelectInput(_display, _Window, ExposureMask | StructureNotifyMask);

    fontinfo = XLoadQueryFont(_display, "10x20");
    gr_values.font = fontinfo->fid;
    gr_values.foreground = XBlackPixel(_display, 0);
    graphical_context = XCreateGC(_display, _Window,GCFont+GCForeground, &gr_values);
    XMapWindow(_display, _Window);
}

bool WindowHandle::Update() {
    XNextEvent(_display, (XEvent *)&_event);
    switch ( _event.type ) {
        case Expose:
        {
            break;
        }
        default:
            break;
    }
    return true;
}

VkSurfaceKHR WindowHandle::createSurface(VkInstance inst) {
    printf("pre create xlib surface\n");
    VkSurfaceKHR surface = nullptr;
    printf("VkSurfaceKHR surface = nullptr\n");
	VkXlibSurfaceCreateInfoKHR createInfo = {};
	printf("VkXlibSurfaceCreateInfoKHR createInfo = {}\n");
	createInfo.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
	printf("createInfo.dpy = _display\n");
	createInfo.dpy = _display;
	printf("createInfo.window = *_Window\n");
	createInfo.window = _Window;
	printf("VK_CHECK_RESULT(vkCreateXlibSurfaceKHR(inst, &createInfo, NULL, &surface))\n");
	VK_CHECK_RESULT(vkCreateXlibSurfaceKHR(inst, &createInfo, NULL, &surface));
	return surface;
}

#endif

WindowHandle::~WindowHandle()
{
}
