/*
	utils.h

	Header file for the utility functions.

	Dmitry Podvigalkin
	
	2025
*/
#pragma once
#include <cstdint>
#include <string>
#include "include.h"

namespace PTE
{

/// <summary>
/// Wrapper class for static helper methods.
/// </summary>
class Utils
{
public:
	static std::wstring GetCurrDir();
	static bool IsElevated();
	static void StopAndDeleteDriver();
	static void ForceCurrentPageToPhysicalMemory(unsigned long long address);
	static void Unpage(const HANDLE hProcess, const LPVOID address);
	static unsigned long InitAndStartDriver();
	static void SendIOCTL(ULONG pid, ULONGLONG address, IOCTL_DATA* data);
	static bool SetDebugPrivilege(const HANDLE hToken);
	static HANDLE OpenSelectedProcessPrivileged();
	static unsigned long long AssembleAddresss(const uint64_t ui1, const uint64_t ui2, const uint64_t ui3, const uint64_t ui4);
	static std::string GetPageData(ULONGLONG virt, LONGLONG phys, const uint8_t* data, size_t len);
	static std::string LargeIntToHexString(const LARGE_INTEGER& li);
};

} //namespace PTE
