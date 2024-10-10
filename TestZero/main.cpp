#include <windows.h>
#include <stdio.h>
#include "../ZeroDriver/ZeroCommon.h"


int Error(const char* msg)
{
	printf("%s: error=%u\n", msg, GetLastError());
	return 1;
}

int main()
{
	HANDLE hDevice = CreateFile(L"\\\\.\\Zero",
		GENERIC_READ | GENERIC_WRITE,
		0, nullptr, OPEN_EXISTING, 0, nullptr);
	if (hDevice == INVALID_HANDLE_VALUE)
	{
		return Error("Failed to open Device\n");
	}

	//test read
	BYTE buffer[64];

	//store some non-zero data
	for (int i = 0; i < sizeof(buffer); i++) {
		buffer[i] = i + 1;
	}

	DWORD bytes;
	BOOL ok = ReadFile(hDevice, buffer, sizeof(buffer), &bytes, nullptr);
	if (!ok) {
		return Error("failed to read\n");
	}

	if (bytes != sizeof(buffer)) {
		Error("Wrong number of bytes\n");
	}

	//check all bytes are zero
	for (auto n : buffer) {
		if (n != 0) {
			printf("Wromg data\n");
			break;
		}
	}

	//test write
	BYTE buffer2[1024];
	ok = WriteFile(hDevice, buffer2, sizeof(buffer2), &bytes, nullptr);
	if (!ok) {
		return Error("failed to write data\n");
	}

	if (bytes != sizeof(buffer2)) {
		return Error("wrong data\n");
	}

	ZeroStats stats;
	if (!DeviceIoControl(hDevice, IOCTL_ZERO_GET_STATUS, nullptr, 0, &stats, sizeof(stats), &bytes, nullptr)) {
		return Error("failed in DeviceIoControl\n");
	}

	printf("Total read: %lld, Total Write: %lld\n", stats.TotalRead, stats.TotalWritten);

	CloseHandle(hDevice);
}