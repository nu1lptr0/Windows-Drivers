#include <ntddk.h>

void SampleUnload(
	PDRIVER_OBJECT DriverObject
) {
	UNREFERENCED_PARAMETER(DriverObject);

	DbgPrint("Sample Driver Unload called\n");
}

extern "C"
NTSTATUS DriverEntry(
	PDRIVER_OBJECT DriverObject,
	PUNICODE_STRING RegistryPath
) {
	
	UNREFERENCED_PARAMETER(RegistryPath);

	DriverObject->DriverUnload = SampleUnload;

	DbgPrint("Sample Driver Initialized successfully!\n");

	return STATUS_SUCCESS;
}