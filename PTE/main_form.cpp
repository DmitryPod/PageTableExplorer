/*
    main_form.cpp

    UI and user mode functionality.
    The UI is implemented using C++/CLI language,
    which is according to Microsoft is "a bridge between
    native C++ code and managed code within the .NET"

    Dmitry Podvigalkin
    
    2025
*/
#include "main_form.h"
#include <Windows.h>
#include <tlhelp32.h>
#include <bitset>
#include <thread>
#include <format>
#include "ia32.hpp"
#include "resource.h"

#pragma comment(lib, "advapi32.lib")

typedef LARGE_INTEGER PHYSICAL_ADDRESS, *PPHYSICAL_ADDRESS;

using namespace System;
using namespace System::Windows::Forms;

/// <summary>
/// Draw a line.
/// </summary>
/// <param name="g">Graphic class</param>
/// <param name="from">Line start coordinate</param>
/// <param name="to">Line end coordinate</param>
/// <param name="arrow">Should the line end with arrow</param>
System::Void PTE::MainForm::DrawLine(Graphics^ g, Point from, Point to, bool isArrow)
{
    Pen^ pen = gcnew Pen(Color::Black, 1.0f);

    if (isArrow)
    {
        pen->CustomEndCap = gcnew Drawing::Drawing2D::AdjustableArrowCap(3.0f, 7.0f);
    }

    g->DrawLine(pen, from, to);
}

/// <summary>
/// Draw UI arrows.
/// </summary>
/// <param name="type">Type of the current table</param>
System::Void PTE::MainForm::DrawArrows(TableType type)
{
    auto gr = panel1->CreateGraphics();

    if (type == TableType::PML4)
    {
        gr->Clear(panel1->BackColor);
    }

    auto cellRectangle = PTs[type]->GetRowDisplayRectangle(
        PTs[type]->CurrentCell->RowIndex, false);

    //
    // Arrows Address -> table offset
    if (type < TableType::PT)
    {
        Point p1(PTs[type]->Location.X + cellRectangle.Right,
            PTs[type]->Location.Y + (cellRectangle.Bottom + cellRectangle.Top) / 2);

        Point p2(PTs[type]->Right + 10, p1.Y);

        Point p4(PTs[type + 1]->Location.X, PTs[type + 1]->Location.Y - 11);
        Point p3(p2.X, p4.Y);

        DrawLine(gr, p1, p2, false);
        DrawLine(gr, p2, p3, false);
        DrawLine(gr, p3, p4, true);
    }

    //
    // Arrows index -> Address
    Point p5((decIndexes[type]->Location.X + decIndexes[type]->Right) / 2,
        decIndexes[type]->Bottom);
    Point p6((decIndexes[type]->Location.X + decIndexes[type]->Right) / 2,
        decIndexes[type]->Bottom + 10);

    Point p7(PTs[type]->Location.X - 20,
        PTs[type]->Location.Y + (cellRectangle.Bottom + cellRectangle.Top) / 2);
    Point p8(PTs[type]->Location.X,
        PTs[type]->Location.Y + (cellRectangle.Bottom + cellRectangle.Top) / 2);

    Point p9(p7.X, p6.Y);
    Point p10(p9.X, p7.Y);

    DrawLine(gr, p5, p6, false);
    DrawLine(gr, p6, p9, false);
    DrawLine(gr, p9, p10, false);
    DrawLine(gr, p7, p8, true);

    //
    // Arrow CR3 Register -> PML4
    if (type == 0)
    {
        Point p11(PTs[type]->Location.X, PTs[type]->Location.Y - 11);
        Point p12(label15->Location.X + 5, PTs[type]->Location.Y - 11);
        Point p13(p12.X, label15->Bottom);

        DrawLine(gr, p13, p12, false);
        DrawLine(gr, p12, p11, true);
    }
}

/// <summary>
/// Draw current table with pages.
/// </summary>
/// <param name="type">Table type</param>
/// <param name="value">Selected table index</param>
/// <param name="physicalAddress">Physical address</param>
System::Void PTE::MainForm::DrawPT(TableType type,
    UInt16 value,
    void* physicalAddress)
{
    auto dataList = gcnew BindingList <PTTable>();
    PHYSICAL_ADDRESS* address = (PHYSICAL_ADDRESS*)(physicalAddress);
    ULONG selectedIndex{ 0 };

    for (uint16_t i = 0; i < TABLE_SIZE; ++i)
    {
        PTTable^ entry;

        if (address[i].QuadPart != 0)
        {        
            entry = gcnew PTTable(i , gcnew String(std::format("0x{:x}", address[i].QuadPart).data()));
            dataList->Add(*entry);
        }
        else if (i == value)
        {
            entry = gcnew PTTable(i, "PAGE FAULT");
            dataList->Add(*entry);
        }

        //
        // Pick the relevant index that would be selected in the UI
        if (i == value)
        {
            selectedIndex = dataList->Count - 1;
        }
    }

    //
    // Bind the data source to the UI table
    this->PTs[type]->DataSource = dataList;

    PTs[type]->Rows[selectedIndex]->Selected = true;
    PTs[type]->CurrentCell = PTs[type]->Rows[selectedIndex]->Cells[0];

    DrawArrows(type);
}

/// <summary>
/// Types accepted by SetPTProperties template method.
/// </summary>
template <typename T>
concept AllowedTempalteTypes = std::is_same_v<T, pml4e_64> ||
                               std::is_same_v<T, pdpte_64> ||
                               std::is_same_v<T, pde_64> ||
                               std::is_same_v<T, pte_64>;
/// <summary>
/// Set the data for the table properties.
/// </summary>
/// <typeparam name="T">The accepted types</typeparam>
/// <param name="value">Properties value as a number</param>
template <AllowedTempalteTypes T>
void SetPTProperties(const uint64_t value)
{
    T val;
    
    //
    // Set the property structure.
    val.flags = value;
    auto pageFrameNumber = gcnew String(std::format("0x{:x}", std::bit_cast<uint64_t>(val.page_frame_number)).data());

    //
    // PT has different set of attributes.
    // Since this is a template method, T type needs to be compile-time evaluated,
    // otherwise, it would error out on type members. 
    if constexpr (std::is_same_v<T, pte_64>)
    {
        PTE::MainForm::GetInstance()->dgvPTProperties->Rows[1]->Cells[1]->Value = val.execute_disable;
        PTE::MainForm::GetInstance()->dgvPTProperties->Rows[1]->Cells[2]->Value = val.protection_key;
        PTE::MainForm::GetInstance()->dgvPTProperties->Rows[1]->Cells[3]->Value = val.ignored_2;
        PTE::MainForm::GetInstance()->dgvPTProperties->Rows[1]->Cells[4]->Value = val.reserved1;
        PTE::MainForm::GetInstance()->dgvPTProperties->Rows[1]->Cells[5]->Value = pageFrameNumber;
        PTE::MainForm::GetInstance()->dgvPTProperties->Rows[1]->Cells[6]->Value = val.ignored_1;
        PTE::MainForm::GetInstance()->dgvPTProperties->Rows[1]->Cells[7]->Value = val.global;
        PTE::MainForm::GetInstance()->dgvPTProperties->Rows[1]->Cells[8]->Value = val.pat;
        PTE::MainForm::GetInstance()->dgvPTProperties->Rows[1]->Cells[9]->Value = val.dirty;
        PTE::MainForm::GetInstance()->dgvPTProperties->Rows[1]->Cells[10]->Value = val.accessed;
        PTE::MainForm::GetInstance()->dgvPTProperties->Rows[1]->Cells[11]->Value = val.page_level_cache_disable;
        PTE::MainForm::GetInstance()->dgvPTProperties->Rows[1]->Cells[12]->Value = val.page_level_write_through;
        PTE::MainForm::GetInstance()->dgvPTProperties->Rows[1]->Cells[13]->Value = val.supervisor;
        PTE::MainForm::GetInstance()->dgvPTProperties->Rows[1]->Cells[14]->Value = val.write;
        PTE::MainForm::GetInstance()->dgvPTProperties->Rows[1]->Cells[15]->Value = val.present;

        PTE::MainForm::GetInstance()->dgvPTProperties->CurrentCell = nullptr;
    }
    else
    {
        //
        // Property table row index.
        uint8_t i = 0;
        if (std::is_same<T, pml4e_64>::value)
        {
            i = 1;
        }
        else if (std::is_same<T, pdpte_64>::value)
        {
            i = 2;
        }
        else if (std::is_same<T, pde_64>::value)
        {
            i = 3;
        }       

        PTE::MainForm::GetInstance()->dgvProperties->Rows[i]->Cells[1]->Value = val.execute_disable;
        PTE::MainForm::GetInstance()->dgvProperties->Rows[i]->Cells[2]->Value = val.ignored_2;
        PTE::MainForm::GetInstance()->dgvProperties->Rows[i]->Cells[3]->Value = val.reserved2;
        PTE::MainForm::GetInstance()->dgvProperties->Rows[i]->Cells[4]->Value = pageFrameNumber;
        PTE::MainForm::GetInstance()->dgvProperties->Rows[i]->Cells[5]->Value = val.ignored_1;
        //
        // "large_page" for pdpte_64 or pde_64. "must_be_zero" for pml4e_64
        PTE::MainForm::GetInstance()->dgvProperties->Rows[i]->Cells[6]->Value = ((value) >> 7) & 0x01;
        PTE::MainForm::GetInstance()->dgvProperties->Rows[i]->Cells[7]->Value = val.reserved1;
        PTE::MainForm::GetInstance()->dgvProperties->Rows[i]->Cells[8]->Value = val.accessed;
        PTE::MainForm::GetInstance()->dgvProperties->Rows[i]->Cells[9]->Value = val.page_level_cache_disable;
        PTE::MainForm::GetInstance()->dgvProperties->Rows[i]->Cells[10]->Value = val.page_level_write_through;
        PTE::MainForm::GetInstance()->dgvProperties->Rows[i]->Cells[11]->Value = val.supervisor;
        PTE::MainForm::GetInstance()->dgvProperties->Rows[i]->Cells[12]->Value = val.write;
        PTE::MainForm::GetInstance()->dgvProperties->Rows[i]->Cells[13]->Value = val.present;

        PTE::MainForm::GetInstance()->dgvProperties->CurrentCell = nullptr;
    }
}

/// <summary>
/// Set Registers table properties.
/// </summary>
/// <param name="response">The data returned by driver.</param>
static void SetRegistersProperties(const IOCTL_RESPONSE& response)
{
    //
    // https://wiki.osdev.org/CPU_Registers_x86
    //
    cr0 cr_0{ .flags = response.regCR0 };
    PTE::MainForm::GetInstance()->dataGridCR0->Rows[1]->Cells[1]->Value = cr_0.reserved4;
    PTE::MainForm::GetInstance()->dataGridCR0->Rows[1]->Cells[2]->Value = cr_0.paging_enable;
    PTE::MainForm::GetInstance()->dataGridCR0->Rows[1]->Cells[3]->Value = cr_0.cache_disable;
    PTE::MainForm::GetInstance()->dataGridCR0->Rows[1]->Cells[4]->Value = cr_0.not_write_through;
    PTE::MainForm::GetInstance()->dataGridCR0->Rows[1]->Cells[5]->Value = cr_0.reserved3;
    PTE::MainForm::GetInstance()->dataGridCR0->Rows[1]->Cells[6]->Value = cr_0.alignment_mask;
    PTE::MainForm::GetInstance()->dataGridCR0->Rows[1]->Cells[7]->Value = cr_0.reserved2;
    PTE::MainForm::GetInstance()->dataGridCR0->Rows[1]->Cells[8]->Value = cr_0.write_protect;
    PTE::MainForm::GetInstance()->dataGridCR0->Rows[1]->Cells[9]->Value = cr_0.reserved1;
    PTE::MainForm::GetInstance()->dataGridCR0->Rows[1]->Cells[10]->Value = cr_0.numeric_error;
    PTE::MainForm::GetInstance()->dataGridCR0->Rows[1]->Cells[11]->Value = cr_0.extension_type;
    PTE::MainForm::GetInstance()->dataGridCR0->Rows[1]->Cells[12]->Value = cr_0.task_switched;
    PTE::MainForm::GetInstance()->dataGridCR0->Rows[1]->Cells[13]->Value = cr_0.emulate_fpu;
    PTE::MainForm::GetInstance()->dataGridCR0->Rows[1]->Cells[14]->Value = cr_0.monitor_coprocessor;
    PTE::MainForm::GetInstance()->dataGridCR0->Rows[1]->Cells[15]->Value = cr_0.protection_enable;
    PTE::MainForm::GetInstance()->dataGridCR0->CurrentCell = nullptr;

    PTE::MainForm::GetInstance()->labelCR2->Text = "CR2: " + gcnew String(std::format("0x{:x}", response.regCR2).data());

    cr3 cr_3{ .flags = response.regCR3 };
    auto address_of_page_directory = cr_3.address_of_page_directory;

    PTE::MainForm::GetInstance()->dataGridCR3->Rows[1]->Cells[1]->Value = cr_3.reserved3;
    PTE::MainForm::GetInstance()->dataGridCR3->Rows[1]->Cells[2]->Value = gcnew String(std::format("0x{:x}", address_of_page_directory).data());
    PTE::MainForm::GetInstance()->dataGridCR3->Rows[1]->Cells[3]->Value = cr_3.reserved2;
    PTE::MainForm::GetInstance()->dataGridCR3->Rows[1]->Cells[4]->Value = cr_3.page_level_cache_disable;
    PTE::MainForm::GetInstance()->dataGridCR3->Rows[1]->Cells[5]->Value = cr_3.page_level_write_through;
    PTE::MainForm::GetInstance()->dataGridCR3->Rows[1]->Cells[6]->Value = cr_3.reserved1;
    PTE::MainForm::GetInstance()->dataGridCR3->CurrentCell = nullptr;

    //
    // TODO: Add CR4
    //

    PTE::MainForm::GetInstance()->dataGridCR0->CurrentCell = nullptr;
    PTE::MainForm::GetInstance()->dataGridCR3->CurrentCell = nullptr;
}

/// <summary>
/// UI updater function.
/// </summary>
/// <param name="sender">Sender info</param>
/// <param name="e">Event info</param>
/// <param name="address">Current memory address</param>
/// <param name="driverResponse">Information about the address</param>
/// <returns></returns>
System::Void PTE::MainForm::UpdateUIBasedOnAddress(unsigned long long address, void* driverResponse)
{
    IOCTL_RESPONSE* response = static_cast<IOCTL_RESPONSE*>(driverResponse);
    bool bPageFault = response->physAddress.QuadPart == 0;

    //
    // Virtual - > Physical addresses.
    labelVirtualAddressHex->Text = gcnew String(std::format("Virtual Address: 0x{:x}:", address).data());
    
    labelBinaryAddress->Text = gcnew String(std::bitset<64>(address).to_string().data());

    String^ physAddr = gcnew String(PTE::Utils::LargeIntToHexString(response->physAddress).data());
    labelPhysicalAddress->Text = "-> Physical Address: " + (bPageFault ? "PAGE FAULT" : physAddr);

    //
    // Update text.
    USHORT b47_39 = static_cast<USHORT>(BYTES_47_39(address));
    USHORT b38_30 = static_cast<USHORT>(BYTES_38_30(address));
    USHORT b29_21 = static_cast<USHORT>(BYTES_29_21(address));
    USHORT b20_12 = static_cast<USHORT>(BYTES_20_12(address));
    USHORT b11_00 = static_cast<USHORT>(BYTES_11_0(address));

    textBox47_39->Text = gcnew String(std::bitset<9>(b47_39).to_string().data());
    textBox38_30->Text = gcnew String(std::bitset<9>(b38_30).to_string().data());
    textBox29_21->Text = gcnew String(std::bitset<9>(b29_21).to_string().data());
    textBox20_12->Text = gcnew String(std::bitset<9>(b20_12).to_string().data());
    textBox11_00->Text = gcnew String(std::bitset<12>(b11_00).to_string().data());

    labelDec47_39->Text = gcnew String(std::to_wstring(b47_39).data());
    labelDec38_30->Text = gcnew String(std::to_wstring(b38_30).data());
    labelDec29_21->Text = gcnew String(std::to_wstring(b29_21).data());
    labelDec20_12->Text = gcnew String(std::to_wstring(b20_12).data());
    labelDec11_00->Text = gcnew String(std::to_wstring(b11_00).data());

    if (!bPageFault || checkBoxAlwaysUnpage->Checked)
    {
        buttonUnpage->Enabled = false;
    }
    else
    {
        buttonUnpage->Enabled = true;
    }

    //
    // Fill the tables
    DrawPT(TableType::PML4, b47_39, &response->pa47_39);
    DrawPT(TableType::PDP, b38_30, &response->pa38_30);
    DrawPT(TableType::PD, b29_21, &response->pa29_21);
    DrawPT(TableType::PT, b20_12, &response->pa20_12);

    //
    // Print bytes.
    if (!bPageFault)
    {
        this->richTextBoxMemory->Text = gcnew String(PTE::Utils::GetPageData(address,
            response->physAddress.QuadPart,
            response->Buffer.data(),
            response->Buffer.size()).data());
    }
    else
    {
        this->richTextBoxMemory->Clear();
    }

    //
    // Address properties
    String^ type;
    String^ state;

    if (s_CurrentProcMemory[address].State == MEM_COMMIT)
    {
        state = gcnew String(" (MEM_COMMIT)");
    }
    else if (s_CurrentProcMemory[address].State == MEM_FREE)
    {
        state = gcnew String(" (MEM_FREE)");
    }
    else if (s_CurrentProcMemory[address].State == MEM_RESERVE)
    {
        state = gcnew String(" (MEM_RESERVE)");
    }
    else
    {
        state = gcnew String("");
    }

    if (s_CurrentProcMemory[address].Type == MEM_IMAGE)
    {
        type = gcnew String(" (MEM_IMAGE)");
    }
    else if (s_CurrentProcMemory[address].Type == MEM_MAPPED)
    {
        type = gcnew String(" (MEM_MAPPED)");
    }
    else if (s_CurrentProcMemory[address].Type == MEM_PRIVATE)
    {
        type = gcnew String(" (MEM_PRIVATE)");
    }

    addressListBox->Items->Clear();
    addressListBox->Items->AddRange(gcnew cli::array< System::Object^>(5)
        {
            gcnew String("Type: " + gcnew String(std::format("0x{:x}", s_CurrentProcMemory[address].Type).data()) + type),
            gcnew String("State: " + gcnew String(std::format("0x{:x}", s_CurrentProcMemory[address].State).data()) + state),
            gcnew String("Protection: " + gcnew String(std::format("0x{:x}", s_CurrentProcMemory[address].Protection).data())),
            gcnew String("Region Size: " + gcnew String(std::format("0x{:x}", s_CurrentProcMemory[address].RegionSize).data())),
            gcnew String("Mapped: " + gcnew String(s_CurrentProcMemory[address].Use.data()))
        }
    );

    //
    // Registers
    SetRegistersProperties(*response);

    // Set the property table
    SetPTProperties<pml4e_64>(response->flagsPML4);
    SetPTProperties<pdpte_64>(response->flagsPDP);
    SetPTProperties<pde_64>(response->flagsPD);
    SetPTProperties<pte_64>(response->flagsPT);
}

/// <summary>
/// Memory cell click handler.
/// </summary>
/// <param name="sender">Sender info</param>
/// <param name="e">Event info</param>
System::Void PTE::MainForm::DgvMemory_CellContentClick(System::Object^ sender, System::Windows::Forms::DataGridViewCellEventArgs^ e)
{
    auto pid = dgvProcesses->CurrentRow->Cells["PID"]->Value;
    ULONGLONG address{ 0 };
   
    //
    // This method could be called in different circumstances.
    // If it was called with the intention to handle a custom address,
    // for example when user clicked on one of the PR tables cells,
    // m_address would contain the desired memory pointer.
    // It would be ideal to pass the address to this function, but it's tricky in C++/CLI event handler.
    if (m_customAddress != 0)
    {
        address = m_customAddress;
        m_customAddress = 0;
    }
    else
    {
        dgvMemory->DefaultCellStyle->SelectionBackColor = System::Drawing::SystemColors::Highlight;

        String^ strAddress{ dgvMemory->CurrentRow->Cells["BaseAddress"]->Value->ToString() };

        //
        // Remove spaces and "0x" and convert to a number
        address = System::UInt64::Parse(strAddress->Substring(strAddress->IndexOf("0x") + 2),
            System::Globalization::NumberStyles::HexNumber);
    }

    //
    // Send request to the driver.
    // The buffer allocated here will be used for both request and response.
    // This memory will be locked by driver while ioctl is processed.
    IOCTL_DATA* ioctlData = static_cast<IOCTL_DATA*>(malloc(sizeof(IOCTL_DATA)));
    if (ioctlData == nullptr)
    {
        return;
    }

    memset(ioctlData, 0, sizeof(IOCTL_DATA));
    
    //
    // Ask driver to give us the info about the address checkBoxAlwaysUnpage->Checked
    bool probe = checkBoxAlwaysUnpage->Checked;
    if (s_UnpageButtonPressed)
    {
        probe = true;
        s_UnpageButtonPressed = false;
    }
    PTE::Utils::SendIOCTL(static_cast<ULONG>(pid), address, ioctlData, probe);

    //
    // Update the UI based on the response from driver
    UpdateUIBasedOnAddress(address, &ioctlData->response);

    if (ioctlData)
    {
        free(ioctlData);
    }
}

/// <summary>
/// Handles any of the page tables click.
/// </summary>
/// <param name="sender">Sender info</param>
/// <param name="e">The event</param>
/// <param name="type">Type of the table</param>
System::Void PTE::MainForm::PTCellClick(System::Object^ sender, System::Windows::Forms::DataGridViewCellEventArgs^ e, const TableType type)
{
    //
    // User clicked on a PT table cell, selected memory address is not longer valid,
    // let's change selector color to emphasise that.
    dgvMemory->DefaultCellStyle->SelectionBackColor = System::Drawing::SystemColors::ControlDark;

    //
    // Get current IDs
    uint64_t ui1 = Convert::ToUInt16(dataGridViewPML4->CurrentRow->Cells["Index"]->Value);
    uint64_t ui2 = 0;
    uint64_t ui3 = 0;
    uint64_t ui4 = 0;

    //
    // Get the indexesx based on the clicked table. 
    if (type >= TableType::PDP)
    {
        ui2 = Convert::ToUInt16(dataGridViewPDP->CurrentRow->Cells["Index2"]->Value);
    }
    if (type >= TableType::PD)
    {
        ui3 = Convert::ToUInt16(dataGridViewPDE->CurrentRow->Cells["Index3"]->Value);
    }
    if (type >= TableType::PT)
    {
        ui4 = Convert::ToUInt16(dataGridViewPT->CurrentRow->Cells["Index4"]->Value);
    }

    //
    // Assemble an address based on the selected cell.
    m_customAddress = Utils::AssembleAddresss(ui1, ui2, ui3, ui4);

    //
    // Trigger the PT tables reprint with the assembled address.
    DataGridViewCellEventArgs^ newEvent = gcnew DataGridViewCellEventArgs(0, dgvMemory->CurrentRow->Index);
    DgvMemory_CellContentClick(sender, newEvent);
}

/// <summary>
/// Table click handler.
/// </summary>
/// <param name="sender">Sender info</param>
/// <param name="e">The event</param>
System::Void PTE::MainForm::DataGridViewPML4_CellClick(System::Object^ sender, System::Windows::Forms::DataGridViewCellEventArgs^ e)
{
    PTCellClick(sender, e, TableType::PML4);
}

/// <summary>
/// Table click handler.
/// </summary>
/// <param name="sender">Sender info</param>
/// <param name="e">The event</param>
System::Void PTE::MainForm::DataGridViewPDP_CellClick(System::Object^ sender, System::Windows::Forms::DataGridViewCellEventArgs^ e)
{
    PTCellClick(sender, e, TableType::PDP);
}

/// <summary>
/// Table click handler.
/// </summary>
/// <param name="sender">Sender info</param>
/// <param name="e">The event</param>
System::Void PTE::MainForm::DataGridViewPDE_CellClick(System::Object^ sender, System::Windows::Forms::DataGridViewCellEventArgs^ e)
{
    PTCellClick(sender, e, TableType::PD);
}

/// <summary>
/// Table click handler.
/// </summary>
/// <param name="sender">Sender info</param>
/// <param name="e">The event</param>
System::Void PTE::MainForm::DataGridViewPT_CellClick(System::Object^ sender, System::Windows::Forms::DataGridViewCellEventArgs^ e)
{
    PTCellClick(sender, e, TableType::PT);
}

/// <summary>
/// Read memory to force-move it to the physical memory.
/// To refresh the UI, emulate the click on the selected memory.
/// </summary>
/// <param name="sender">Sender info.</param>
System::Void PTE::MainForm::ButtonUnpage_Click(System::Object^ sender, System::EventArgs^ /*e*/)
{
    s_UnpageButtonPressed = true;
    //
    // Trigger cell click again to update the values.
    DataGridViewCellEventArgs^ newEvent = gcnew DataGridViewCellEventArgs(0, dgvMemory->CurrentRow->Index);
    DgvMemory_CellContentClick(sender, newEvent);
}

/// <summary>
/// Create a memory table entry and get the memory properties.
/// </summary>
/// <param name="mbi">Memory information</param>
/// <param name="strAddress">The Memory address as a string.</param>
/// <returns>Memory table entry</returns>
static PTE::AddressTable^ GetAddressEntry(const MEMORY_BASIC_INFORMATION& mbi, const std::string& strAddress)
{
    std::wstring result;

    if (mbi.Protect & PAGE_EXECUTE)
    {
        result = L"X";
    }
    else if (mbi.Protect & PAGE_EXECUTE_READ)
    {
        result = L"RX";
    }
    else if (mbi.Protect & PAGE_EXECUTE_READWRITE)
    {
        result = L"RWX";
    }
    else if (mbi.Protect & PAGE_EXECUTE_WRITECOPY)
    {
        result = L"WCX";
    }
    else if (mbi.Protect & PAGE_NOACCESS)
    {
        result = L"NA";
    }
    else if (mbi.Protect & PAGE_READONLY)
    {
        result = L"R";
    }
    else if (mbi.Protect & PAGE_READWRITE)
    {
        result = L"RW";
    }
    else if (mbi.Protect & PAGE_WRITECOPY)
    {
        result = L"WC";
    }
    else if (mbi.Protect & PAGE_TARGETS_INVALID)
    {
        result = L"INV";
    }
    else if (mbi.Protect & PAGE_TARGETS_NO_UPDATE)
    {
        result = L"NU";
    }
    else if (mbi.State & MEM_RESERVE)
    {
        result = L"RESERVE";
    }
    else
    {
        result = L"";
    }

    if (mbi.Protect & PAGE_GUARD)
    {
        result += L",PG";
    }
    if (mbi.Protect & PAGE_NOCACHE)
    {
        result += L",NC";

    }
    if (mbi.Protect & PAGE_WRITECOMBINE)
    {
        result+= L",WCM";
    }

    //
    // Create the memory table entry
    auto tableEntry = gcnew PTE::AddressTable(gcnew String(strAddress.data()), gcnew String(result.data()));

    if ((mbi.State & MEM_RESERVE))
    {
        tableEntry->CellColor = System::Drawing::SystemColors::ControlLight;
    }
    else if (mbi.Protect & PAGE_GUARD)
    {
        tableEntry->CellColor = System::Drawing::Color::Plum;
    }
    else
    {
        switch (mbi.Type)
        {
        case MEM_IMAGE:
            tableEntry->CellColor = System::Drawing::SystemColors::GradientActiveCaption;
            break;
        case MEM_MAPPED:
            tableEntry->CellColor = System::Drawing::Color::PeachPuff;
            break;
        case MEM_PRIVATE:
            break;
        default:
            tableEntry->CellColor = System::Drawing::Color::Transparent;
            break;
        }
    }

    return tableEntry;
}

/// <summary>
/// Fill the address table for the selected process.
/// </summary>
/// <param name="e">The event</param>
System::Void PTE::MainForm::DrawAddressTable(System::Object^ /*sender*/, System::Windows::Forms::DataGridViewCellEventArgs^ e)
{
    if (e->RowIndex < 0)
    {
        return;
    }

    //
    // s_CurrentProcMemory hash table is used to store the memory and its properties,
    // so it could be retrieved when user clicks on an address without requesting the properties.
    dgvMemory->DefaultCellStyle->SelectionBackColor = System::Drawing::SystemColors::Highlight;
    s_CurrentProcMemory.clear();
    stripStatusLabel->Text = "";

    //
    // Data source for the table
    auto memoryList = gcnew BindingList <AddressTable>();
 
    HANDLE hProcess = PTE::Utils::OpenSelectedProcessPrivileged();
    if (!hProcess)
    {
        return;
    }

    MEMORY_BASIC_INFORMATION mbi;
    uint8_t* address = 0;

    while (VirtualQueryEx(hProcess, address, &mbi, sizeof(mbi)))
    {
        address += mbi.RegionSize;

        //
        // Include base addresses to show the hierarchy
        if ((!(mbi.State & MEM_COMMIT) && mbi.AllocationBase != mbi.BaseAddress) ||
            mbi.Protect == PAGE_NOACCESS)
        {
            continue;
        }

        //
        // Indent if not a base address.
        std::string strAddress{ };
        if (mbi.AllocationBase != mbi.BaseAddress)
        {
            strAddress += std::format("   ");
        }
        strAddress += std::format("0x{:x}", std::bit_cast<ULONGLONG>(mbi.BaseAddress));

        //
        // Set the map of addresses.
        // It is used to get the memory properties when user clicks of an address.
        s_CurrentProcMemory[std::bit_cast<ULONG_PTR>(mbi.BaseAddress)] =
        {
            .Type = mbi.Type,
            .Protection = mbi.Protect,
            .State = mbi.State,
            .RegionSize = mbi.RegionSize
        };

        //
        // If backed by file, get its name.
        if (mbi.Type == MEM_IMAGE)
        {
            wchar_t wzModuleName[MAX_PATH]{ };

            GetModuleFileNameW(std::bit_cast<HMODULE>(mbi.AllocationBase), wzModuleName, MAX_PATH);
            s_CurrentProcMemory[std::bit_cast<ULONG_PTR>(mbi.BaseAddress)].Use = wzModuleName;
        }

        memoryList->Add(*GetAddressEntry(mbi, strAddress));
    }

    if (hProcess)
    {
        CloseHandle(hProcess);
    }

    dgvMemory->DataSource = memoryList;

    //
    // Set the table rows colors based on the data.
    // Unsigned index is preferable, but Count is signed so...
    for (int i = 0; i < memoryList->Count; ++i)
    {
        if (memoryList[i].CellColor != System::Drawing::Color::Transparent)
        {
            dgvMemory->Rows[i]->DefaultCellStyle->BackColor = memoryList[i].CellColor;
        }
    }
}

/// <summary>
/// Unpage memory by reading a single byte from the address.
/// </summary>
System::Void PTE::MainForm::ForceCurrentPageToPhysicalMemory()
{
    //
    // We could take the address from memory table,
    // but if user clicked on any of the page table cells it won't be accurate.
    // So instead, take the address from the selected cells.
    uint64_t ui1 = Convert::ToUInt16(dataGridViewPML4->CurrentRow->Cells["Index"]->Value);
    uint64_t ui2 = Convert::ToUInt16(dataGridViewPDP->CurrentRow->Cells["Index2"]->Value);
    uint64_t ui3 = Convert::ToUInt16(dataGridViewPDE->CurrentRow->Cells["Index3"]->Value);
    uint64_t ui4 = Convert::ToUInt16(dataGridViewPT->CurrentRow->Cells["Index4"]->Value);

    //
    // Assemble a fake address
    m_customAddress = Utils::AssembleAddresss(ui1, ui2, ui3, ui4);
}

/// <summary>
/// Draw the table of process addresses.
/// </summary>
System::Void PTE::MainForm::DgvProcesses_CellContentClick(System::Object^ sender, System::Windows::Forms::DataGridViewCellEventArgs^ e)
{
    DrawAddressTable(sender, e);
}

/// <summary>
/// Refresh the process table.
/// </summary>
System::Void PTE::MainForm::RefreshButton_Click(System::Object^ /*sender*/, System::EventArgs^ /*e*/)
{
    EnumProcesses();
}

/// <summary>
/// Enumerate all the processes running on the system
/// and display them as a [pid, name] table.
/// </summary>
System::Void PTE::MainForm::EnumProcesses()
{
    //
    // Not using DataTable or binding list data source here to take advantage
    // of the default sorting capabilities of the table.
    PROCESSENTRY32W proc{ .dwSize = sizeof(PROCESSENTRY32W) };

    HANDLE hProcess = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hProcess == INVALID_HANDLE_VALUE)
    {
        stripStatusLabel->Text = "Failed to enumerate processes!";
        return;
    }

    if (!Process32FirstW(hProcess, &proc))
    {
        stripStatusLabel->Text = "Failed to enumerate processes!";
        goto Exit;
    }

    dgvProcesses->Rows->Clear();

    do
    {
        dgvProcesses->Rows->Add(proc.th32ProcessID, gcnew String(proc.szExeFile));
    } while (Process32NextW(hProcess, &proc));

Exit:

    CloseHandle(hProcess);
}

/// <summary>
/// Get the icon from the running process.
/// The process gets it from the Resources.
/// </summary>
System::Void PTE::MainForm::SetFormIcon()
{
    HMODULE hModule = ::GetModuleHandle(NULL);
    HICON hIcon = ::LoadIconW(hModule, MAKEINTRESOURCE(IDI_ICON1));

    this->Icon = System::Drawing::Icon::FromHandle(System::IntPtr(hIcon));
}

/// <summary>
/// Run the thread with GUI.
/// </summary>
void RunGuiThread()
{
    Application::Run(PTE::MainForm::GetInstance());
}

/// <summary>
/// Application entry.
/// </summary>
int main()
{
    Application::EnableVisualStyles();
    Application::SetCompatibleTextRenderingDefault(false);

    PTE::MainForm form;

    std::thread thread(RunGuiThread); 

    thread.join();

    return 0;
}
