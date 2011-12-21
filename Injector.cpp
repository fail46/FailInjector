#include "FailInjector.hpp"

byte LoadLibraryABytes[56] = {0x8B, 0xFF, 0x55, 0x8B, 0xEC, 0x83, 0x7D, 0x08, 0x00, 0x53, 0x56, 0x57, 0x74, 0x17, 0x68, 0x00, 0x4C, 0x4B, 0x76, 0xFF, 0x75, 0x08, 0xE8, 0x31, 0x00, 0x00, 0x00, 0x59, 0x59, 0x85, 0xC0, 0x0F, 0x84, 0xD7, 0xEE, 0x02, 0x00, 0x6A, 0x00, 0x6A, 0x00, 0xFF, 0x75, 0x08, 0xE8, 0xC8, 0xD1, 0xFF, 0xFF, 0x5F, 0x5E, 0x5B, 0x5D, 0xC2, 0x04, 0x00};

void Injector::Inject (unsigned int ProcessID, const char* ModulePath)
{
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
	
	// This should find most hooks on LoadLibraryA
	byte Bytes[56];
	ReadProcessMemory(Handle, LoadLibraryAddress, Bytes, 56, nullptr);

	unsigned int i = 0;
	while(i < 56)
	{
		if(LoadLibraryABytes[i] != Bytes[i])
		{
			if(MessageBox(UI::MainWindow, "The function LoadLibraryA appears to be hooked or otherwise modified in the remote process. This could result in the target process detecting the injection. Inject anyway?", "Function Modified", MB_YESNO) != IDYES)
			{
				return;
			}
			else
			{
				break;
			}
		}

		i++;
	}

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
	return;
}