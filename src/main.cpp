#include "engine.h"
#include <iostream>
#include <string>

int main()
{
	Engine();
	std::string name;
#ifdef _DEBUG
	getline(std::cin, name);
#endif // DEBUG
}
