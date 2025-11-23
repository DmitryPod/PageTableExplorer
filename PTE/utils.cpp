/*
    utils.cpp
    
    Utility methods.

    Dmitry Podvigalkin
    
    2025
*/
#include "main_form.h"
#include <iostream>
#include <bitset>
#include <format>
#include <sstream>
#include <iomanip>

static const std::wstring s_wstrDriverName = L"PTE_Driver";
static const std::wstring s_wstrDriverPath = PTE::Utils::GetCurrDir() + L"\\PTE_Driver.sys";

/// <summary>
/// Get current directory path.
/// </summary>
/// <returns>Path</returns>
std::wstring PTE::Utils::GetCurrDir()
{
    std::wstring wstrPath(MAX_PATH, 0);
    GetModuleFileNameW(NULL, wstrPath.data(), MAX_PATH);
    return wstrPath.substr(0, wstrPath.find_last_of(L"\\/"));
}

/// <summary>
/// Converts large integer to hex string
/// </summary>
/// <param name="li">Input</param>
/// <returns>Input as hex string</returns>
std::string PTE::Utils::LargeIntToHexString(const LARGE_INTEGER& li)
{
    std::stringstream ss;
    ss << std::hex << std::setw(12) << std::setfill('0') << li.QuadPart;
    return "0x" + ss.str();
}

/// <summary>
/// Get a page content as a formatted string.
/// </summary>
/// <param name="virt">Virtual address</param>
/// <param name="phys">Physical address</param>
/// <param name="data">Data blob</param>
/// <param name="len">The blob size.</param>
/// <returns>The string with the memory information</returns>
std::string PTE::Utils::GetPageData(ULONGLONG virt, LONGLONG phys, const uint8_t* data, size_t len)
{
    const short sAllignBy = 16;
    std::string strResult;
    std::string strAscii;

    for (int i = 0; i < len; ++i)
    {
        //
        // Add current address if this is the new line.
        if (i % sAllignBy == 0)
        {
            strResult += std::format("0x{:012x}", virt + i);
            strResult += " -> ";
            strResult += std::format("0x{:012x} : ", phys + i);
        }

        strResult += std::format("{:02X} ", data[i]);
        if (data[i] < 0x20)
        {
            strAscii += '.';
        }
        else
        {
            strAscii += data[i];
        }

        //
        // New line
        if ((i + 1) % sAllignBy == 0)
        {
            strResult += " ";
            strResult += strAscii;
            strResult += '\n';
            strAscii.clear();
        }
    }

    return strResult;
}

/// <summary>
/// Use the tables indexes to get the memory address.
/// </summary>
/// <param name="ui1">Index PML4</param>
/// <param name="ui2">Index PDP</param>
/// <param name="ui3">Index PD</param>
/// <param name="ui4">Index PT</param>
/// <returns>Memory address.</returns>
unsigned long long PTE::Utils::AssembleAddresss(const uint64_t ui1,
    const uint64_t ui2,
    const uint64_t ui3,
    const uint64_t ui4)
{
    unsigned long long result{ 0 };

    result += (ui1 << 39);
    result += (ui2 << 30);
    result += (ui3 << 21);
    result += (ui4 << 12);

    return result;
}

/// <summary>
/// Read 1 byte from the provided address.
/// </summary>
/// <param name="hProcess">Process handle</param>
/// <param name="address">Memory address</param>
void PTE::Utils::Unpage(const HANDLE hProcess, const LPVOID address)
{
    //
    // Read a single byte to force address into the physical memory
    uint8_t buffer;

    std::string strAddress = std::format("0x{:x}", static_cast<ULONGLONG>(PTE::MainForm::GetInstance()->GetCustomAddress()));

    if (ReadProcessMemory(hProcess, address, std::bit_cast<LPVOID>(&buffer), 1, nullptr) == 0)
    {
        PTE::MainForm::GetInstance()->stripStatusLabel->Text = gcnew String("Failed to Unpage: [" + gcnew String(strAddress.data()) + "]");
    }
    else
    {
        PTE::MainForm::GetInstance()->stripStatusLabel->Text = gcnew String("Unpaged successfully: [" + gcnew String(strAddress.data()) + "]");
    }
}


/// <summary>
/// Try to open the process handle, apply debug privileges if necessary.
/// </summary>
/// <returns>Handle to the process or nullptr</returns>
HANDLE PTE::Utils::OpenSelectedProcessPrivileged()
{
    DWORD pid = static_cast<ULONG>(PTE::MainForm::GetInstance()->dgvProcesses->CurrentRow->Cells["PID"]->Value);

    //
    // System process cannot be handled.
    if (pid <= 4)
    {
        PTE::MainForm::GetInstance()->dgvMemory->Rows->Clear();
        return nullptr;
    }

    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    if (hProcess)
    {
        return hProcess;
    }

    if (GetLastError() != ERROR_ACCESS_DENIED)
    {
        PTE::MainForm::GetInstance()->stripStatusLabel->Text = "Cannot get the handle to the process! Try running tool as Administrator.";
        return nullptr;
    }

    //
    // If didn't open because of the privileges, try to open with privileges.
    if (!PTE::MainForm::GetInstance()->IsDebugPrivileged())
    {
        //
        // Pseudo handle need not be closed
        HANDLE hCurrentProcess = GetCurrentProcess();
        HANDLE hToken = nullptr;

        //
        // Try giving us debug privileges
        if (OpenProcessToken(hCurrentProcess, TOKEN_ADJUST_PRIVILEGES, &hToken) == TRUE)
        {
            if (PTE::Utils::SetDebugPrivilege(hToken))
            {
                PTE::MainForm::GetInstance()->SetDebugPrivileged();
            }
            else
            {
                PTE::MainForm::GetInstance()->stripStatusLabel->Text = "Failed to get debug privileges:" + GetLastError();
            }
            CloseHandle(hToken);
        }
    }

    //
    // Try regardless of outcome, most likely will fail without debug privileges.
    hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
    if (!hProcess)
    {
        PTE::MainForm::GetInstance()->stripStatusLabel->Text = "Cannot get the handle to the process. Try running tool as Administrator.";
    };

    return hProcess;
}

/// <summary>
/// Attempt to register and start the driver.
/// </summary>
/// <returns>True on success</returns>
unsigned long PTE::Utils::InitAndStartDriver()
{
    unsigned long result{ 0 };
    
    SC_HANDLE hManager = OpenSCManagerW(NULL, NULL, SC_MANAGER_CREATE_SERVICE);
    if (hManager == nullptr)
    {
        return GetLastError();
    }
    
    SC_HANDLE hService = CreateServiceW(hManager,
        s_wstrDriverName.data(),
        s_wstrDriverName.data(),
        GENERIC_EXECUTE,
        SERVICE_KERNEL_DRIVER,
        SERVICE_SYSTEM_START,
        SERVICE_ERROR_NORMAL,
        s_wstrDriverPath.data(),
        L"NDIS",
        nullptr,
        L"NDIS\0",
        nullptr,
        nullptr);
    if (hService == nullptr && GetLastError() != ERROR_SERVICE_EXISTS)
    {
        result = GetLastError();
        goto Exit;
    }

    //
    // If already running for some reason, try to re-open.
    if (hService == nullptr)
    {
        hService = OpenServiceW(hManager, s_wstrDriverName.data(), READ_CONTROL | SERVICE_START);
        if (hService == nullptr)
        {
            result = GetLastError();
            goto Exit;
        }
    }

    if (StartServiceW(hService, 0, nullptr) != TRUE && 
        GetLastError() != ERROR_SERVICE_ALREADY_RUNNING)
    {
        result = GetLastError();
        goto Exit;
    }

Exit:
    if (hService)
    {
        CloseServiceHandle(hService);
    }   
    if (hManager)
    {
        CloseServiceHandle(hManager);
    }

    return result;
}

/// <summary>
/// Attempt to stop and unregister the driver.
/// </summary>
/// <returns>True on success</returns>
void PTE::Utils::StopAndDeleteDriver()
{
    SERVICE_STATUS serviceStatus = { 0 };

    //
    // Do not attempt to output anything as we're terminating anyway.
    SC_HANDLE hManager = OpenSCManagerW(NULL, NULL, SC_MANAGER_CONNECT);
    if (hManager == nullptr)
    {
        return;
    }

    SC_HANDLE hService = OpenServiceW(hManager, s_wstrDriverName.data(), DELETE | SERVICE_STOP);
    if (hService == nullptr)
    {
        goto Exit;
    }

    if (!DeleteService(hService))
    {
        goto Exit;
    }

    ControlService(hService, SERVICE_CONTROL_STOP, &serviceStatus);

Exit:
    if (hService)
    {
        CloseServiceHandle(hService);
    }
    if (hManager)
    {
        CloseServiceHandle(hManager);
    }
}

/// <summary>
/// Check if the application is running as administrator.
/// </summary>
/// <returns>True if elevated.</returns>
bool PTE::Utils::IsElevated()
{
    bool result{ false };
    HANDLE hToken = nullptr;
    if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken))
    {
        TOKEN_ELEVATION tokenInfo;
        DWORD size = sizeof(TOKEN_ELEVATION);
        if (GetTokenInformation(hToken, TokenElevation, &tokenInfo, sizeof(tokenInfo), &size))
        {
            result = (tokenInfo.TokenIsElevated != 0);
        }
    }
    if (hToken)
    {
        CloseHandle(hToken);
    }
    return result;
}

/// <summary>
/// Set debug privileges for a process.
/// </summary>
/// <param name="hToken">Process token</param>
/// <returns>True on success.</returns>
bool PTE::Utils::SetDebugPrivilege(const HANDLE hToken)
{
    LUID luid;

    if (LookupPrivilegeValueW(NULL, SE_DEBUG_NAME, &luid) != TRUE)
    {
        return false;
    }

    TOKEN_PRIVILEGES tp;

    tp.PrivilegeCount = 1;
    tp.Privileges[0].Luid = luid;
    tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    if (AdjustTokenPrivileges(hToken, FALSE, &tp, 0, nullptr, nullptr) != TRUE)
    {
        return false;
    }

    return GetLastError() == ERROR_SUCCESS;
}

/// <summary>
/// Send request to the driver to get information about a memory address.
/// </summary>
/// <param name="pid">Process id</param>
/// <param name="address">The memory address we're interested in</param>
/// <param name="data">The request and the response data</param>
/// <param name="probe">Ask driver to page in the data</param>
void PTE::Utils::SendIOCTL(ULONG pid, ULONGLONG address, IOCTL_DATA* data, bool probe)
{
    HANDLE device = INVALID_HANDLE_VALUE;

    data->address = std::bit_cast<PVOID>(address);
    data->pid = pid;
    data->probe = probe;

    device = CreateFileW(L"\\\\.\\PTEDeviceLink",
        GENERIC_WRITE | GENERIC_READ | GENERIC_EXECUTE,
        0,
        0,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_SYSTEM,
        0);
    if (device == INVALID_HANDLE_VALUE)
    {
        PTE::MainForm::GetInstance()->stripStatusLabel->Text = "Could not open device.";
        return;
    }

    if (!DeviceIoControl(device, IOCTL_CODE, data, sizeof(*data), data, sizeof(*data), nullptr, nullptr))
    {
        PTE::MainForm::GetInstance()->stripStatusLabel->Text = gcnew String("IOCTL failed: [" + GetLastError() + "]");
    }

    CloseHandle(device);

    return;
}
