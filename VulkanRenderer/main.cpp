#include "Engine.h"
#include <iostream>
#include <string>

#define VULKAN_HPP_NO_EXCEPTIONS

int main()
{
	Engine();
	std::string name;
#ifdef _DEBUG
	getline(std::cin, name);
#endif // DEBUG
}