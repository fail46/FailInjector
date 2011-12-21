#include "FailInjector.hpp"

int __stdcall WinMain (HINSTANCE Instance, HINSTANCE PreviousInstance, char* CommandLine, int ShowCommand)
{
	UI::Create(Instance);
	return 0;
}