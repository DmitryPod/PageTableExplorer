/*!
	pte_driver.cpp
  
	Page Table Driver - handles the IOCTLs from the User Land.
  
	Dmitry Podvigalkin
	
	2025
 
	This driver will be initialized and 

	Some useful driver commands:
 		sc create PTE_Driver type= kernel binPath= <path_to_driver>
		sc start PTE_Driver
		sc stop PTE_Driver
		sc delete PTE_Driver

	Tables hierarchy: 
		PML4 -> PDP -> PD -> PT -> RAM
		PML4:	Page Map Level 4
		PDP:	Page Directory Pointers
		PD:		Page Directories
		PT:		Page Tables

	Each process will have its own Page directory and tables,
	so the same virtual address may get translated into
	different physical addresses depending on the current process context.
 */
#define _HAS_EXCEPTIONS 1

#include <fltKernel.h>
#include <array>
#include <intrin.h>
#include "include.h"
#include "pte_driver.h"
#pragma warning( push )
#pragma warning( disable : 4201 )
#include "..\Common\ia32.hpp"
#pragma warning( pop )

/// <summary>
/// Driver entry point.
/// </summary>
/// <param name="driverObject">Pointer to driver object</param>
/// <param name="registryPath">The driver's configuration information in the registry.</param>
/// <returns>Status</returns>
NTSTATUS DriverEntry(_In_ PDRIVER_OBJECT driverObject, _In_ PUNICODE_STRING registryPath)
{
	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, ("Driver Loaded\n"));
	return Ioctl::RegisterIoctl(driverObject, registryPath);
}

/// <summary>
/// Called before the system unloads the driver.
/// </summary>
/// <param name="driverObject">Pointer to driver object</param>
void DriverUnload(PDRIVER_OBJECT driverObject)
{
	IoDeleteDevice(driverObject->DeviceObject);
	IoDeleteSymbolicLink(&g_DeviceSymbName);
	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, ("Driver Unloaded\n"));
}

/// <summary>
/// Supplies the functions handled by the driver.
/// </summary>
/// <returns>Status</returns>
NTSTATUS MajorFunctions(PDEVICE_OBJECT /*deviceObject*/, PIRP /*irp*/)
{
	return STATUS_SUCCESS;
}

/// <summary>
/// Registers the Ioctl handled by the driver.
/// </summary>
/// <param name="deviceObject">Pointer to driver object</param>
/// <returns>Status</returns>
NTSTATUS Ioctl::RegisterIoctl(PDRIVER_OBJECT driverObject, PUNICODE_STRING /*registryPath*/)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL;

	driverObject->DriverUnload = DriverUnload;
	driverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = Ioctl::HandleIOCTL;
	driverObject->MajorFunction[IRP_MJ_CREATE] = MajorFunctions;
	driverObject->MajorFunction[IRP_MJ_CLOSE] = MajorFunctions;

	status = IoCreateDevice(driverObject,
		0,
		&g_DeviceName,
		FILE_DEVICE_UNKNOWN,
		FILE_DEVICE_SECURE_OPEN,
		FALSE,
		&driverObject->DeviceObject);
	if (!NT_SUCCESS(status))
	{
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "Could not create device %wZ", g_DeviceName);
		return status;
	}

	status = IoCreateSymbolicLink(&g_DeviceSymbName, &g_DeviceName);
	if (!NT_SUCCESS(status))
	{
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "Error creating symbolic link %wZ", g_DeviceName);
		return status;
	}

	return STATUS_SUCCESS;
}

/// <summary>
/// Constructor for a scoped MDL class.
/// </summary>
/// <param name="irp">I/O request packet</param>
Ioctl::MdlScoped::MdlScoped(PIRP irp)
{
	if (m_MDL != nullptr)
	{
		NT_ASSERT(m_MDL == nullptr);
		return;
	}

	//
	// Lock User Mode memory with MDL in case the UM app goes away
	// while we're working with the memory
	m_MDL = IoAllocateMdl(irp->AssociatedIrp.SystemBuffer,
		sizeof(IOCTL_DATA),
		FALSE,
		FALSE,
		nullptr);
	if (m_MDL == nullptr)
	{
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "MDL allocation failed.");
		return;
	}

	MmProbeAndLockPages(m_MDL, KernelMode, IoWriteAccess);
}

/// <summary>
/// Destructor for a scoped MDL class.
/// </summary>
Ioctl::MdlScoped::~MdlScoped()
{
	if (m_MDL == nullptr)
	{
		return;
	}

	MmUnlockPages(m_MDL);
	IoFreeMdl(m_MDL);
	m_MDL = nullptr;
}

/// <summary>
/// Handle Ioctl from the user mode.
/// </summary>
/// <param name="deviceObject">Pointer to driver object</param>
/// <param name="irp">I/O request packet</param>
/// <returns>Status</returns>
NTSTATUS Ioctl::HandleIOCTL(PDEVICE_OBJECT /*deviceObject*/, PIRP irp)
{
	NTSTATUS status{ STATUS_UNSUCCESSFUL };
	IOCTL_DATA* request{ nullptr };

	PIO_STACK_LOCATION stackLocation = IoGetCurrentIrpStackLocation(irp);

	if (stackLocation->Parameters.DeviceIoControl.IoControlCode != IOCTL_CODE ||
		irp->AssociatedIrp.SystemBuffer == nullptr)
	{
		return status;
	}

	MdlScoped mdl(irp);

	//
	// Now get the address to work with
	request = std::bit_cast<IOCTL_DATA*>(MmGetSystemAddressForMdlSafe(mdl.Get(), NormalPagePriority));
	if (request == nullptr)
	{
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "Failed to get MDL address.\n");
	}
	else
	{
		GetDataForAddress(request->pid, request->address, request->response);
		irp->IoStatus.Information = sizeof(*request);
		status = STATUS_SUCCESS;
	}

	irp->IoStatus.Status = status;
	IoCompleteRequest(irp, IO_NO_INCREMENT);

	return status;
}

/// <summary>
/// Check if supplied address is valid.
/// </summary>
/// <param name="address">The target address</param>
/// <returns>True if valid</returns>
bool Ioctl::IsAddressValid(void* address)
{
	return address && MmIsAddressValid(address);
}

/// <summary>
/// Gather the data for the supplied memory address.
/// </summary>
/// <param name="address">The address of interest</param>
/// <param name="data">The structure used for the output</param>
void Ioctl::AnalyzeAddress(PVOID address, IOCTL_RESPONSE& data)
{
	ULONG_PTR ullAddress = std::bit_cast<ULONGLONG>(address);

	USHORT b47_39 = static_cast<USHORT>(BYTES_47_39(ullAddress));
	USHORT b38_30 = static_cast<USHORT>(BYTES_38_30(ullAddress));
	USHORT b29_21 = static_cast<USHORT>(BYTES_29_21(ullAddress));
	USHORT b20_12 = static_cast<USHORT>(BYTES_20_12(ullAddress));

	//
	// Get final physical address
	data.physAddress = MmGetPhysicalAddress(address);

	//
	// Get the kernel's page directory
	cr3 cr3;
	cr3.flags = __readcr3();

	//
	// Get the rest of the registers data
	data.regCR0 = __readcr0();
	data.regCR2 = __readcr2();
	data.regCR3 = cr3.flags;

	PHYSICAL_ADDRESS physAddress;

	//
	// PML4 
	//

	// 
	// Get physical address of PML4 table
	physAddress.QuadPart = cr3.address_of_page_directory << PAGE_SHIFT;

	//
	// Get virtual address of PML4 table
	pml4e_64* virtPML4 = reinterpret_cast<pml4e_64*>(MmGetVirtualForPhysical(physAddress));
	if (!IsAddressValid(virtPML4))
	{
		return;
	}

	data.flagsPML4 = virtPML4->flags;

	//
	// Iterate through the table and collect valid indexes
	for (auto i = 0; i < TABLE_SIZE; i++)
	{
		if (!virtPML4[i].present)
		{
			continue;
		}

		PHYSICAL_ADDRESS physPML4{ 0 };
		physPML4.QuadPart = virtPML4[i].page_frame_number << PAGE_SHIFT;
		data.pa47_39[i] = physPML4;
	}

	//
	// PDP
	//
	
	if (!virtPML4[b47_39].present)
	{
		return;
	}

	//
	// Convert page frame number to a physical address.
	// Get physical address of the PDP table.
	physAddress.QuadPart = virtPML4[b47_39].page_frame_number << PAGE_SHIFT;

	//
	// Get virtual address of PDP table
	pdpte_64* virtPDP = reinterpret_cast<pdpte_64*>(MmGetVirtualForPhysical(physAddress));
	if (!IsAddressValid(virtPDP))
	{
		return;
	}

	data.flagsPDP = virtPDP->flags;

	for (auto i = 0; i < TABLE_SIZE; i++)
	{
		if (!virtPDP[i].present)
		{
			continue;
		}

		PHYSICAL_ADDRESS physPDP{ 0 };
		physPDP.QuadPart = virtPDP[i].page_frame_number << PAGE_SHIFT;

		data.pa38_30[i] = physPDP;
	}

	//
	// PD
	//
	
	if (!virtPDP[b38_30].present)
	{
		return;
	}

	// Get physical address of the PD table.
	physAddress.QuadPart = virtPDP[b38_30].page_frame_number << PAGE_SHIFT;

	// Get virtual address of PD table
	pde_64* virtPD = reinterpret_cast<pde_64*>(MmGetVirtualForPhysical(physAddress));
	if (!IsAddressValid(virtPD))
	{
		return;
	}

	data.flagsPD = virtPD->flags;

	for (auto i = 0; i < TABLE_SIZE; i++)
	{
		if (!virtPD[i].present)
		{
			continue;
		}
		PHYSICAL_ADDRESS physPD{ 0 };
		physPD.QuadPart = virtPD[i].page_frame_number << PAGE_SHIFT;
		data.pa29_21[i] = physPD;
	}

	//
	// PT
	//

	if (!virtPD[b29_21].present)
	{
		return;
	}

	//
	// Get physical address of the PT table.
	physAddress.QuadPart = virtPD[b29_21].page_frame_number << PAGE_SHIFT;

	//
	// Get virtual address of PT table
	pte_64* virtPT = reinterpret_cast<pte_64*>(MmGetVirtualForPhysical(physAddress));
	if (!IsAddressValid(virtPT))
	{
		return;
	}

	data.flagsPT = virtPT->flags;

	for (auto i = 0; i < TABLE_SIZE; i++)
	{
		if (!virtPT[i].present)
		{
			continue;
		}

		PHYSICAL_ADDRESS physPT{ 0 };
		physPT.QuadPart = virtPT[i].page_frame_number << PAGE_SHIFT;
		data.pa20_12[i] = physPT;
	}

	//
	// At this point we went through all 4 tables.
	// We enumerated all the values in the last PT (Page Table) table.
	// We don't need to get phys and virt addresses for virtPD[b20_11] because
	// MmGetPhysicalAddress at the very beginning already got the final address
	if (!virtPT[b20_12].present)
	{
		return;
	}

	//
	// Copy the data in the page
	if (IsAddressValid(address))
	{
		memcpy(&data.Buffer, address, PAGE_SIZE);
	}
}

/// <summary>
/// Get information about the virtual address
/// </summary>
/// <param name="ulPID">Process id</param>
/// <param name="address">The address of interest</param>
/// <param name="data">The structure used for the output</param>
void Ioctl::GetDataForAddress(ULONG ulPID, PVOID address, IOCTL_RESPONSE& data)
{
	//
	// Ignore system processes
	if (ulPID <= 4)
	{
		return;
	}

	//
	// Get process handle with privileges.
	HANDLE hProcess = ULongToHandle(ulPID);
	PEPROCESS pe = nullptr;

	NTSTATUS status = PsLookupProcessByProcessId(hProcess, &pe);
	if (!NT_SUCCESS(status) || pe == nullptr)
	{
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "Failed to find the process [%lu].\n", ulPID);
		return;
	}

	status = ObOpenObjectByPointer(pe,
		OBJ_KERNEL_HANDLE,
		nullptr,
		PROCESS_ALL_ACCESS,
		*PsProcessType,
		KernelMode,
		&hProcess);
	if (!NT_SUCCESS(status))
	{
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, ("Error opening PEPROCESS\n"));
		return;
	}

	//
	// We need to be in the context of the process.
	KAPC_STATE state{ };
	KeStackAttachProcess(pe, &state);

	AnalyzeAddress(address, data);

	KeUnstackDetachProcess(&state);

	ZwClose(hProcess);
}
