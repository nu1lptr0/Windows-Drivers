#include <ntifs.h>
#include <ntddk.h>
#include "BoosterCommon.h"

void BoosterUnload(PDRIVER_OBJECT DriverObject)
{
	UNICODE_STRING symlink;
	
	RtlInitUnicodeString(&symlink, L"\\?\\Booster");

	//delete symbolic link
	IoDeleteSymbolicLink(&symlink);

	//delete device object
	IoDeleteDevice(DriverObject->DeviceObject);
}

NTSTATUS BoosterCreateClose(PDEVICE_OBJECT DeviceObject, PIRP Irp)
{
	UNREFERENCED_PARAMETER(DeviceObject);

	Irp->IoStatus.Status = STATUS_SUCCESS;
	Irp->IoStatus.Information = 0;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);

	return STATUS_SUCCESS;
}

NTSTATUS BoosterWrite(PDEVICE_OBJECT DeviceObject, PIRP Irp)
{
	UNREFERENCED_PARAMETER(DeviceObject);
	auto status = STATUS_SUCCESS;
	ULONG_PTR information = 0;

	//irpSp is of type PIO_STACK_LOCATION
	auto irpSp = IoGetCurrentIrpStackLocation(Irp);

	do {
		if (irpSp->Parameters.Write.Length < sizeof(ThreadData)) {
			status = STATUS_BUFFER_TOO_SMALL;
			break;
		}

		auto data = static_cast<ThreadData*>(Irp->UserBuffer);
		if (data == nullptr) {
			status = STATUS_INVALID_PARAMETER;
			break;
		}
		__try {

			if (data->priority < 1 || data->priority >31) {
				status = STATUS_INVALID_PARAMETER;
				break;
			}
			PETHREAD thread;
			status = PsLookupThreadByThreadId(ULongToHandle(data->ThreadId), &thread);
			if (!NT_SUCCESS(status)) {
				DbgPrint("Failed to LookUp for the thread\n");
				break;
			}

			auto oldPriority = KeSetPriorityThread(thread, data->priority);
			DbgPrint("Priority changed for thread %u from %d to %d succeeded!\n", data->ThreadId, oldPriority, data->priority);

			//decrement the refrence count to the thread
			ObDereferenceObject(thread);

			information = sizeof(data);
		}
		__except (GetExceptionCode() == STATUS_ACCESS_VIOLATION
			? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
			//handle exception
		}
		
	} while (false);

	//Complete the IRP with status we got at this point.
	Irp->IoStatus.Status = status;
	Irp->IoStatus.Information = information;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	
	return status;
}

extern "C" 
NTSTATUS DriverEntry(
	PDRIVER_OBJECT DriverObject,
	PUNICODE_STRING RegistryPath
)
{
	UNICODE_STRING devName;
	UNICODE_STRING symLink;
	PDEVICE_OBJECT DeviceObject;
	UNREFERENCED_PARAMETER(RegistryPath);

	DriverObject->MajorFunction[IRP_MJ_CREATE] = BoosterCreateClose;
	DriverObject->MajorFunction[IRP_MJ_CLOSE] = BoosterCreateClose;
	DriverObject->MajorFunction[IRP_MJ_WRITE] = BoosterWrite;

	DriverObject->DriverUnload = BoosterUnload;

	RtlInitUnicodeString(&devName, L"\\Device\\Booster");
	RtlInitUnicodeString(&symLink, L"\\??\\Booster");

	NTSTATUS status = IoCreateDevice(
		DriverObject,
		0,
		&devName,
		FILE_DEVICE_UNKNOWN,
		0,
		FALSE,
		&DeviceObject);
	
	if (!NT_SUCCESS(status))
	{
		DbgPrint("failed to create device object(0x%08X)\n", status);
		return status;
	}

	NT_ASSERT(DeviceObject);

	status = IoCreateSymbolicLink(&symLink, &devName);
	if (!NT_SUCCESS(status))
	{
		DbgPrint("Failed to create symbolic Link (0x%08X)\n", status);
		IoDeleteDevice(DeviceObject);
		return status;
	}

	NT_ASSERT(NT_SUCCESS(status));

	return STATUS_SUCCESS;
}