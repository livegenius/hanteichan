#include "framestate.h"
#include "main.h"
#include <windef.h>
#include <winbase.h>
#include <sstream>
#include <linear_allocator.hpp>

constexpr const wchar_t *sharedMemHandleName = L"hanteichan-shared_mem";

FrameState::FrameState()
{
	SYSTEM_INFO sInfo;
	GetSystemInfo(&sInfo);
	size_t bufSize = 0x100*sInfo.dwAllocationGranularity; //16MB should be plenty
	size_t appendSize = (1+sizeof(CopyData)/sInfo.dwAllocationGranularity)*sInfo.dwAllocationGranularity;

	sharedMemHandle = CreateFileMapping(
		INVALID_HANDLE_VALUE,    // use paging file
		NULL,                    // default security
		PAGE_READWRITE,          // read/write access
		0,                       // maximum object size (high-order DWORD)
		bufSize+appendSize,                // maximum object size (low-order DWORD)
		sharedMemHandleName);   // name of mapping object

	auto exists = GetLastError();

	void *baseAddress = (void*)((size_t)sInfo.lpMinimumApplicationAddress+sInfo.dwAllocationGranularity*0x5000);

	sharedMem = MapViewOfFileEx(
		sharedMemHandle,
		FILE_MAP_ALL_ACCESS,
		0,
		0,
		bufSize+appendSize,
		baseAddress);

	if(sharedMemHandle == nullptr || sharedMem == nullptr)
	{
		std::wstringstream ss;
		ss << L"Error code: " << GetLastError() << L"\n"
			L"Handle: "<<sharedMemHandle<<L"\nMap: "<<sharedMem<<L"\n\nTry using an older version, \"bro\".";
		MessageBox(mainWindowHandle, ss.str().c_str(), L"Shared memory mapping error", MB_ICONERROR);
		abort();
	}

	ta_init(sharedMem, (char*)sharedMem+bufSize, 65535, 256, 16, exists);
	
	if(!exists)
		copied = new((char*)sharedMem+bufSize) CopyData;
	else
		copied = reinterpret_cast<CopyData*>((char*)sharedMem+bufSize);
}

FrameState::~FrameState()
{
	//copied->~CopyData();
	UnmapViewOfFile(sharedMem);
	CloseHandle(sharedMemHandle);
}