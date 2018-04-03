#pragma once

#include <string>
#if defined(_WIN32)
#include <windows.h>
#else
#include<X11/Xlib.h>
#endif
#include <vulkan/vulkan.h>

class Engine;

class WindowHandle
{
public:
	WindowHandle(uint32_t size_x, uint32_t size_y, std::string title, Engine* engine);
	~WindowHandle();

	bool Update();

	void Close() { bisOpen = false; };

	VkSurfaceKHR createSurface(VkInstance inst);
#if defined(_WIN32)
	HWND get() { return hWnd; }

private:
	HWND hWnd;
	HINSTANCE hInstance;


	std::string							_win32_class_name;
	static uint64_t						_win32_class_id_counter;
#else

    Display* _display;
    Window _Window;
    XKeyEvent  _event;

#endif // Xlib
private:
    bool bisOpen = true;
};

