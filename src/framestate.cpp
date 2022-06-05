#include "framestate.h"
#include "main.h"
#include <windef.h>
#include <winbase.h>
#include <sstream>
#include <linear_allocator.hpp>

constexpr const wchar_t *sharedMemHandleName = L"hanteichan-shared_mem";

FrameState::FrameState()
{
	constexpr size_t bufSize = 0x1000000; //32MB should be plenty

	sharedMemHandle = CreateFileMapping(
		INVALID_HANDLE_VALUE,    // use paging file
		NULL,                    // default security
		PAGE_READWRITE,          // read/write access
		0,                       // maximum object size (high-order DWORD)
		bufSize,                // maximum object size (low-order DWORD)
		sharedMemHandleName);                 // name of mapping object

	sharedMem = MapViewOfFile(
		sharedMemHandle,
		FILE_MAP_ALL_ACCESS,
		0,
		0,
		bufSize);

	if(sharedMemHandle == nullptr || sharedMem == nullptr)
	{
		std::wstringstream ss;
		ss << L"Error code: " << GetLastError() << L"\n"
			L"Handle: "<<sharedMemHandle<<L"\nMap: "<<sharedMem<<L"\n\nTry using an older version, \"bro\".";
		MessageBox(mainWindowHandle, ss.str().c_str(), L"Shared memory mapping error", MB_ICONERROR);
		abort();
	}

	ta_init(sharedMem, (char*)sharedMem+bufSize, 65535, 256, 8);
}

FrameState::~FrameState()
{
	UnmapViewOfFile(sharedMem);
	CloseHandle(sharedMemHandle);
}