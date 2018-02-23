#pragma once

#include <string>
#include <windows.h>
#include <vulkan/vulkan.hpp>
#include <vulkan/vk_sdk_platform.h>

class Engine;

class Window
{
public:
	Window(uint32_t size_x, uint32_t size_y, std::string title, Engine* engine);
	~Window();

	bool Update();

	void Close() { bisOpen = false; };

	HWND get() { return hWnd; }

	vk::SurfaceKHR createSurface(vk::Instance inst);

private:
	HWND hWnd;
	HINSTANCE hInstance;
	bool bisOpen = true;

	std::string							_win32_class_name;
	static uint64_t						_win32_class_id_counter;
};

