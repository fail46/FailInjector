#include "FailInjector.hpp"

string ModulePath;
string ModuleName;
unsigned int ProcessID;

HWND UI::MainWindow;
HWND UI::ProcessList;
HWND UI::InjectButton;
HWND UI::SelectModuleButton;
HWND UI::Selected;

void __stdcall UI::Create (HINSTANCE Instance)
{
	WNDCLASSEX FIClass;
	ZeroMemory(&FIClass, sizeof(WNDCLASSEX));

    MSG Message;
    FIClass.cbSize        = sizeof(WNDCLASSEX);
	FIClass.lpfnWndProc   = UI::MainUIProc;
    FIClass.hInstance     = Instance;
    FIClass.hCursor       = LoadCursor(0, IDC_ARROW);
	FIClass.hIcon		  = LoadIcon(Instance, MAKEINTRESOURCE(IDI_FI));
	FIClass.hIconSm		  = LoadIcon(Instance, MAKEINTRESOURCE(IDI_FI));
    FIClass.hbrBackground = (HBRUSH)COLOR_WINDOW;
    FIClass.lpszClassName = "FailInjector";
	RegisterClassEx(&FIClass);

	MainWindow = CreateWindowEx(
		WS_EX_CLIENTEDGE,
		"FailInjector",
		"FailInjector",
		WS_SYSMENU,
		CW_USEDEFAULT, CW_USEDEFAULT, 206, 115,
		nullptr, nullptr, Instance, nullptr);

	UpdateWindow(MainWindow);
	ShowWindow(MainWindow, SW_SHOW);

	while(GetMessage(&Message, 0, 0, 0) > 0)
	{
		TranslateMessage(&Message);
		DispatchMessage(&Message);
	}

	return;
}

long __stdcall UI::MainUIProc (HWND Window, unsigned int Message, WPARAM wParam, LPARAM lParam)
{
	try
	{
		switch(Message)
		{
		case WM_CREATE:
			Selected = CreateWindowEx(0, "static", "No Module -> No Process",
				WS_CHILD | WS_VISIBLE | SS_CENTER,
				2, 65, 196, 25, Window, nullptr, nullptr, nullptr);
			SendMessage(Selected, WM_SETFONT, reinterpret_cast<WPARAM>(GetStockObject(DEFAULT_GUI_FONT)), 0);

			SelectModuleButton = CreateWindowEx(0, "button", "Select Module",
				WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
				4, 30, 93, 30, Window, reinterpret_cast<HMENU>(1), nullptr, nullptr);
			SendMessage(SelectModuleButton, WM_SETFONT, reinterpret_cast<WPARAM>(GetStockObject(DEFAULT_GUI_FONT)), 0);

			InjectButton = CreateWindowEx(0, "button", "Inject",
				WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
				101, 30, 93, 30, Window, reinterpret_cast<HMENU>(2), nullptr, nullptr);
			SendMessage(InjectButton, WM_SETFONT, reinterpret_cast<WPARAM>(GetStockObject(DEFAULT_GUI_FONT)), 0);

			ProcessList = CreateWindowEx(0, "combobox", "Process List",
				WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST,
				5, 5, 142, 20, Window, nullptr, nullptr, nullptr);
			SendMessage(ProcessList, WM_SETFONT, reinterpret_cast<WPARAM>(GetStockObject(DEFAULT_GUI_FONT)), 0);

			SendMessage(CreateWindowEx(0, "button", "Refresh",
					WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
					150, 4, 45, 22, Window, reinterpret_cast<HMENU>(3), nullptr, nullptr),
				WM_SETFONT, reinterpret_cast<WPARAM>(GetStockObject(DEFAULT_GUI_FONT)), 0);

			UpdateProcessList();
			break;

		case WM_COMMAND:
			CommandHandler(LOWORD(wParam), HIWORD(wParam));
			break;

		case WM_CLOSE:
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		}
	}
	catch(exception exc)
	{
		char* Exception = new char[strlen(exc.what()) + 14];
		sprintf(Exception, "%s Error: %d", exc.what(), GetLastError());

		MessageBox(Window, Exception, nullptr, MB_OK);
	}

	return DefWindowProc(Window, Message, wParam, lParam);
}

void __stdcall UI::CommandHandler (unsigned int Command, unsigned int High)
{
	switch(Command)
	{
	case 1:
	{
		char File[MAX_PATH] = {0};

		OPENFILENAME OpenFileName;
		ZeroMemory(&OpenFileName, sizeof(OpenFileName));

		OpenFileName.hwndOwner = MainWindow;
		OpenFileName.lpstrFilter = "DLLs\0*.dll\0Executables\0*.exe\0";
		OpenFileName.nFilterIndex = 1;
		OpenFileName.lpstrFile = File;
		OpenFileName.nMaxFile = MAX_PATH;
		OpenFileName.lpstrTitle = "Select a Module";
		OpenFileName.nFileExtension = 2;
		OpenFileName.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
		OpenFileName.lStructSize = sizeof(OPENFILENAME);
		
		if(GetOpenFileName(&OpenFileName) == 0)
		{
			return;
		}

		ModulePath.clear();
		ModulePath.append(File);
		PathStripPath(File);
		ModuleName.clear();
		ModuleName.append(File);
		break;
	}

	case 2:
		if(ProcessID != 0 && ModulePath.length() != 0)
		{
			Injector::Inject(ProcessID, ModulePath.c_str());
			UpdateProcessList();
		}
		break;

	case 3:
		UpdateProcessList();
		break;
	}

	unsigned int CurrentSelection = SendMessage(ProcessList, CB_GETCURSEL, 0, 0);
	char* ListText[100];
	if(CurrentSelection != -1)
	{
		SendMessage(ProcessList, CB_GETLBTEXT, CurrentSelection, reinterpret_cast<LPARAM>(ListText));
	}

	char PIDString[6];
	if(ListText != nullptr)
	{
		memcpy(PIDString, ListText, 5);
		ProcessID = atoi(PIDString);
	}

	string NewSelectedText;
	if(ModuleName.length() > 4)
	{
		NewSelectedText.append(ModuleName);
	}
	else
	{
		NewSelectedText.append("No Module");
	}

	NewSelectedText.append(" -> ");

	char PID[12];
	if(ProcessID != 0)
	{
		_itoa_s(ProcessID, PID, 10);
	}
	else
	{
		strcpy_s(PID, "No Process");
	}

	NewSelectedText.append(PID);
	SendMessage(Selected, WM_SETTEXT, 0, reinterpret_cast<LPARAM>(NewSelectedText.c_str()));
	
	return;
}

void UI::UpdateProcessList ()
{
	DWORD Processes[301];
	EnumProcesses(Processes, 1200, nullptr);

	unsigned int CurrentSelection = SendMessage(ProcessList, CB_GETCURSEL, 0, 0);
	char* OldText[100];

	if(CurrentSelection != -1)
	{
		SendMessage(ProcessList, CB_GETLBTEXT, CurrentSelection, reinterpret_cast<LPARAM>(OldText));
	}

	char OldPIDString[6];
	unsigned int OldPID = 0;
	if(OldText != nullptr)
	{
		memcpy(OldPIDString, OldText, 5);
		OldPID = atoi(OldPIDString);
	}

	string Process;
	char ModuleName[26];
	SendMessage(ProcessList, CB_RESETCONTENT, 0, 0);

	unsigned int ProcessIDPosition = 0;
	unsigned int i = 1;
	while(i <= 300 && Processes[i] != 0)
	{
		if(OldPID == Processes[i])
		{
			ProcessIDPosition = i;
		}

		char PID[12];
		_itoa_s(Processes[i], PID, 10);
		Process.append(PID);
		Process.append(" - ");

		void* Handle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, false, Processes[i]);
		if(GetModuleBaseName(Handle, 0, ModuleName, 25) == 0)
		{
			CloseHandle(Handle);
			Process.clear();
			i++;
			continue;
		}

		CloseHandle(Handle);
		Process.append(ModuleName);
		SendMessage(ProcessList, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(Process.c_str()));
		Process.clear();
		i++;
	}

	if(ProcessIDPosition != -1)
	{
		SendMessage(ProcessList, CB_SETCURSEL, ProcessIDPosition, 0);
	}

	return;
}