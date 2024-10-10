### Process
- A process is a containment and management object that represents a running instance of a program. Process doesn't run - processes manages.
- it has a private virtual address space, used for allocating memory needed by the code within the process.
- A *primary token * which is an object that stores the security context of the process.
- A private handle table to events , semaphores and files.
- One or more  threads of execution.

### Virtual Memory
- For 64 bit process on 64 bit windows system , the address space is 128 TB.
- The memory is called *virtual memory* which means there is an indirect relationship between an address and  the exact location where its ca be found on the RAM.
- The unit of memory management is called *page*.

### Threads
- The actual entities that execute the code are threads . A Thread is contained in the process , using the resources exposed by the process to do work.
- Current access mode -> usermode or KernelMode.
- Base priority and processor priority.
- States of threads are -> Running(Currently executing on a processor), ready(waiting to be scheduled for execution), and Waiting( waiting for some event to occur before procedding).

### General System Architecture
- User Processes
- **Subsystem DLLS** -> These are the DLLS that implement the API of subsystem. These are well known files such as kernel32.dll, user32.dll, gdi32.dll, combase.dll and many others.
- **NTDLL.DLL** -> A system wide DLL , implementing the windows native API. NTDLL also implements the Heap Manager, Image Loader ...
- **Service Processes** -> Normal windows Processes that communicate with the service control Manager(SCM).
- **Execurtive** -> The executive is the upper layer of the Ntoskrnl.exe . It includes various managers : Object Manager , Memory maanager ...
- **kernel** -> This includes thread scheduling, interrupt and exception handling, mutexes and semaphores.
- **Win32k.sys** -> It is a kernel driver that handles the user interface part of windows and Graphics Device Interface(GDI) APIs.
- **HAL** -> The HAL is a software abstraction layer over the hardware close to the cpu. This layer is usefuul to device drivers written to handle hardware devices.
-

### Handles and Objects
- The windows kernel exposes various typesof objects for use by usermode and kernel mode itself.Instances of this type of data structures are stored in system space, created by Object Manager.Since, these indexes resides in the system space, usermode can't access it directly .
- Objects are reference Count.
- Usermode uses an indirect mechanism to access objects , called handles.A handle is an index to an entry in a table maintained on a process by process basis, stored in kernel space, thats points to a kernel Object residing in the system space.
- There are various *Create* and *Open* functions to create/open objects and retrieve back the handles to these objects.
- Kernel and driver code can either use an handle or direct pointer to an object . Handle given by user mdoe to driver can be turned into pointer using *ObReferenceObjectByHandle* API .
- Handles values are multiples of 4.
- Each object points to  an object type , which hols the information on the type itself.

### Object Names
- 


### Driver Object
- The important set of operations to initialize are called *Dispatch Routines*. These are array of function pointers stored in the MajorFunction member of DRIVER_OBJECT.
- *IRP_MJ_CREATE, IRP_MJ_CLOSE, IRP_MJ_READ, IRP_MJ_WRITE* -> used for Create, Close,Read and write APIs.
- *IRP_MJ_INTERNAL_DEVICE_CONTROL* -> Same as IRP_MJ_DEVICE_CONTROL but only available for kernel-mode callers.
- *IRP_MJ_SHUTDOWN* -> called when the system shutdowns.
- *IRP_MJ_CLEANUP* -> Used to clean the reference , when the last handle to object is closed.
- IRP_MJ_PNP, IRP_MJ_POWER -> Called by PNP manager and Power manager.

### Kernel Mechanisms
- **IRQL** -> Every hardware Interrupt is associated with priority, called Interrupt Request level(IRQL).  