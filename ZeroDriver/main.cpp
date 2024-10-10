#include "pch.h"

#define DRIVER_PREFIX "Zero: "

long long g_TotalRead;
long long g_TotalWritten;

NTSTATUS CompleteIrp(PIRP Irp,
	NTSTATUS status = STATUS_SUCCESS,
	ULONG_PTR info = 0) {
	Irp->IoStatus.Status = status;
	Irp->IoStatus.Information = info;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return status;
}

//Unload routine
void ZeroUnload(PDRIVER_OBJECT DriverObject)
{
	UNICODE_STRING symLink;
	RtlInitUnicodeString(&symLink, L"\\??\\Zero");
	IoDeleteSymbolicLink(&symLink);
	IoDeleteDevice(DriverObject->DeviceObject);
}

//Create and close dispatch routine
NTSTATUS ZeroCreateClose(PDEVICE_OBJECT DeviceObject, PIRP Irp) {
	UNREFERENCED_PARAMETER(DeviceObject);
	return CompleteIrp(Irp);
}

//Read Dispatch Routine
NTSTATUS ZeroRead(PDEVICE_OBJECT DeviceObject, PIRP Irp) {

	UNREFERENCED_PARAMETER(DeviceObject);
	auto stack = IoGetCurrentIrpStackLocation(Irp);
	auto len = stack->Parameters.Read.Length;
	if (len == 0)
		return CompleteIrp(Irp, STATUS_INVALID_BUFFER_SIZE);

	NT_ASSERT(Irp->MdlAddress);     // Make sure DIRECT_IO flag was set
	auto buffer = MmGetSystemAddressForMdlSafe(Irp->MdlAddress, NormalPagePriority);
	if (!buffer) {
		return CompleteIrp(Irp, STATUS_INSUFFICIENT_RESOURCES);
	}

	memset(buffer, 0, len);

	//update the number of bytes read
	InterlockedAdd64(&g_TotalRead, len);

	return CompleteIrp(Irp, STATUS_SUCCESS, len);
}

//Write dispatch routine 
NTSTATUS ZeroWrite(PDEVICE_OBJECT DeviceObject, PIRP Irp)
{
	UNREFERENCED_PARAMETER(DeviceObject);
	auto stack = IoGetCurrentIrpStackLocation(Irp);
	auto len = stack->Parameters.Write.Length;

	//update the number of bytes written
	InterlockedAdd64(&g_TotalWritten, len);
	return CompleteIrp(Irp, STATUS_SUCCESS, len);
}

// DeviceControl Dispatch routine
NTSTATUS ZeroDeviceControl(
	PDEVICE_OBJECT DeviceObject,
	PIRP Irp
) {
	auto irpSp = IoGetCurrentIrpStackLocation(Irp);
	auto& dic = irpSp->Parameters.DeviceIoControl;
	auto status = STATUS_INVALID_DEVICE_REQUEST;
	ULONG_PTR len = 0;

	switch (dic.IoControlCode) {
	case IOCTL_ZERO_GET_STATUS:
	{
		if (dic.OutputBufferLength < sizeof(ZeroStats)) {
			status = STATUS_BUFFER_TOO_SMALL;
			break;
		}

		auto stats = (ZeroStats*)Irp->AssociatedIrp.SystemBuffer;
		if (stats == nullptr) {
			status = STATUS_INVALID_PARAMETER;
			break;
		}

		//fill in the output buffer
		stats->TotalRead = g_TotalRead;
		stats->TotalWritten = g_TotalWritten;
		len = sizeof(ZeroStats);

		//change  status to indicate success
		status = STATUS_SUCCESS;
		break;
	}

	case IOCTL_ZERO_CLEAR_STATUS:
		g_TotalRead = g_TotalWritten = 0;
		status = STATUS_SUCCESS;
		break;
	}

	return CompleteIrp(Irp, status, len);
}

//DriverEntry

extern "C" 
NTSTATUS DriverEntry(
	PDRIVER_OBJECT DriverObject,
	PUNICODE_STRING RegistryPath
) {
	
	UNREFERENCED_PARAMETER(RegistryPath);
	UNICODE_STRING devName;
	UNICODE_STRING symLink;

	DriverObject->DriverUnload = ZeroUnload;

	DriverObject->MajorFunction[IRP_MJ_CREATE] = ZeroCreateClose;
	DriverObject->MajorFunction[IRP_MJ_CLOSE] = ZeroCreateClose;
	DriverObject->MajorFunction[IRP_MJ_READ] = ZeroRead;
	DriverObject->MajorFunction[IRP_MJ_WRITE] = ZeroWrite;
	DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = ZeroDeviceControl;

	RtlInitUnicodeString(&devName, L"\\Device\\Zero");
	RtlInitUnicodeString(&symLink, L"\\??\\Zero");

	PDEVICE_OBJECT DeviceObject = nullptr;
	auto status = STATUS_SUCCESS;

	do {
		status = IoCreateDevice(DriverObject, 0, &devName, FILE_DEVICE_UNKNOWN, 0, FALSE, &DeviceObject);
		if (!NT_SUCCESS(status)) {
			DbgPrint(DRIVER_PREFIX "Failed to create device (0x%08X)\n", status);
			break;
		}

		//setup DIRECT_IO
		DeviceObject->Flags |= DO_DIRECT_IO;

		//create symlink
		status = IoCreateSymbolicLink(&symLink, &devName);
		if (!NT_SUCCESS(status)) {
			DbgPrint(DRIVER_PREFIX "Failed to create symbolic link (0x%08X)\n", status);
			break;
		}

	} while (false);

	if (!NT_SUCCESS(status)) {
		if (DeviceObject) {
			IoDeleteDevice(DeviceObject);
		}
	}

	return status;
}