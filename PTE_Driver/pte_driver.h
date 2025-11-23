/*
	pte_driver.h

	Driver header file.
	Handles the IOCTL from the user space application.

	Dmitry Podvigalkin
	
	2025
*/
#pragma once
#include <wdm.h>

static UNICODE_STRING g_DeviceName = RTL_CONSTANT_STRING(L"\\Device\\PTEDevice");
static UNICODE_STRING g_DeviceSymbName = RTL_CONSTANT_STRING(L"\\??\\PTEDeviceLink");
extern "C"
{
	DRIVER_INITIALIZE DriverEntry;
	void DriverUnload(PDRIVER_OBJECT driverObject);
	NTSTATUS MajorFunctions(PDEVICE_OBJECT deviceObject, PIRP irp);
}

class Ioctl
{
public:
	static NTSTATUS RegisterIoctl(PDRIVER_OBJECT driverObject, PUNICODE_STRING registryPath);

	static NTSTATUS HandleIOCTL(PDEVICE_OBJECT deviceObject, PIRP irp);

	static void AnalyzeAddress(PVOID address, IOCTL_RESPONSE& data);

	static void GetDataForAddress(ULONG pid, PVOID address, IOCTL_RESPONSE& data, bool probe);

	static bool IsAddressValid(void* address);

private:
	//
	// Wrapper for MDL to manage memory
	class MdlScoped
	{
	public:
		MdlScoped(PVOID address, SIZE_T size, LOCK_OPERATION lockType);

		~MdlScoped();

		PMDL Get()
		{
			return m_MDL;
		}

	private:
		 PMDL m_MDL = nullptr;
	};
};


