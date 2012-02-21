#include "FailInjector.hpp"

void Injector::Inject (unsigned int ProcessID, const char* ModulePath)
{
	void* Token = nullptr;
	TOKEN_PRIVILEGES Privileges;
	if(OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, reinterpret_cast<void**>(&Token)) == 0)
	{
		throw exception("Unable to open the local process token.");
	}

	Privileges.PrivilegeCount = 1;
	LookupPrivilegeValue(nullptr, "SeDebugPrivilege", &Privileges.Privileges[0].Luid);
	Privileges.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	AdjustTokenPrivileges(Token, 0, &Privileges, sizeof(Privileges), nullptr, nullptr);
	CloseHandle(Token);
	
	void* Handle = OpenProcess(PROCESS_ALL_ACCESS, false, ProcessID);
	if(Handle == nullptr)
	{
		throw exception("Unable to open a handle to the target process.");
	}

	void* RemoteString = VirtualAllocEx(Handle, nullptr, 520, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	if(WriteProcessMemory(Handle, RemoteString, ModulePath, 260, nullptr) == 0)
	{
		CloseHandle(Handle);
		throw exception("Unable to write module path into target process.");
	}

	void* LoadLibraryAddress = GetProcAddress(GetModuleHandle("kernel32.dll"), "LoadLibraryA");
	
	void* Thread = CreateRemoteThread(Handle, nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(LoadLibraryAddress), RemoteString, 0, nullptr);
	if(Thread == nullptr)
	{
		CloseHandle(Thread);
		throw exception("Unable to create thread in target process.");
	}

	if(WaitForSingleObject(Thread, 1000) == WAIT_TIMEOUT)
	{
		CloseHandle(Handle);
		CloseHandle(Thread);
		throw exception("Remote thread timed out. Injection may have failed.");
	}

	CloseHandle(Thread);
	VirtualFreeEx(Handle, RemoteString, 520, MEM_FREE);
	CloseHandle(Handle);
	UI::UpdateProcessList();
	return;
}