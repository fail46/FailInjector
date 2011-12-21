#pragma once

// Windows
#include <Windows.h>
#include <Psapi.h>
#include <Shlwapi.h>

// C++ Standard Library
#include <exception>
#include <string>
#include <vector>

using std::exception;
using std::string;
using std::vector;

#pragma comment(lib, "Psapi.lib")
#pragma comment(lib, "Shlwapi.lib")

// FailInjector
#include "Resource.h"

#define null 0
typedef unsigned char byte;

namespace UI
{
	extern HWND MainWindow;
	extern HWND ProcessList;
	extern HWND InjectButton;
	extern HWND SelectModuleButton;
	extern HWND Selected;

	extern char* Module;
	extern char* Process;

	void __stdcall Create (HINSTANCE Instance);
	long __stdcall MainUIProc (HWND Window, unsigned int Message, WPARAM wParam, LPARAM lParam);
	void __stdcall CommandHandler (unsigned int Command, unsigned int High);
	void UpdateProcessList ();
}

namespace Injector
{
	void Inject (unsigned int ProcessID, const char* ModulePath);
}