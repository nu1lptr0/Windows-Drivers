#include <Windows.h>
#include <stdio.h>
#include "../Booster/BoosterCommon.h"

int Error(const char* message)
{
	printf("%s (error=%u)\n", message, GetLastError());
	return 1;
}

int main(int argc, char* argv[])
{
	if (argc < 3)
	{
		printf("Usage: Boost <threadid> <priority>\n");
		return 0;
	}

	/*
	* extract from the command line
	*/
	int tid = atoi(argv[1]);
	int priority = atoi(argv[2]);

	/*Open handle to the device
	*/
	HANDLE hDevice = CreateFile(L"\\\\.\\Booster", GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, 0, nullptr);
	if (hDevice == INVALID_HANDLE_VALUE)
	{
		return Error("Failed to open device\n");
	}

	ThreadData data;
	data.ThreadId = tid;
	data.priority = priority;

	DWORD returned;
	BOOL success = WriteFile(hDevice,
		&data, sizeof(data),
		&returned, nullptr);
	if (!success)
	{
		return Error("Priority change failed\n");
	}

	printf("Priority Changed succeeded!\n");

	CloseHandle(hDevice);

	return 0;
}