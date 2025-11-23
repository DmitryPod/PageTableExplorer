/*
	main_form.h

	The main form header file.
	The UI is implemented using C++/CLI language,
	which is according to Microsoft is "a bridge between 
	native C++ code and managed code within the .NET"

	Dmitry Podvigalkin
	
	2025
*/
#pragma once
#include "Utils.h"
#include <unordered_map>

typedef struct _MEMORY_INFORMATION
{
	DWORD Type;
	DWORD Protection;
	DWORD  State;
	SIZE_T RegionSize;
	std::wstring Use;
} MEMORY_INFORMATION, *PMEMORY_INFORMATION;

namespace PTE
{
using namespace System;
using namespace System::ComponentModel;
using namespace System::Collections;
using namespace System::Windows::Forms;
using namespace System::Data;
using namespace System::Drawing;
using namespace System::Drawing;

/// <summary>
/// The map contains the data about the current addresses.
/// Ideally this should live within the class, but since C++/CLI
/// doesn't make it easy, defining this as a global.
/// </summary>
static std::unordered_map<ULONG_PTR, MEMORY_INFORMATION> s_CurrentProcMemory{};

//
// Properties names should match column names.
// Used for data binding.
public value struct PTTable
{
public:
	//
	// Public properties
	property UInt16 Index;
	property String^ Value;

	//
	// Constructor
	PTTable(UInt16 a, String^ b)
	{
		Index = a;
		Value = b;
	}
};	
	
//
// Properties names should match column names.
public value struct ProcessTable
{
public:
	// Public properties
	property UInt32 PID;
	property String^ ProcessName;

	// Constructor
	ProcessTable(UInt32 a, String^ b)
	{
		PID = a;
		ProcessName = b;
	}
};	

public value struct AddressTable
{
public:
	//
	// Public properties
	property String^ BaseAddress;
	property String^ Protection;

	Color CellColor;

	//
	// Constructor
	AddressTable(String^ a, String^ b)
	{
		BaseAddress = a;
		Protection = b;
	}
};
	
//
// Table indexes.
public enum TableType : int
{
	PML4 = 0,
	PDP,
	PD,
	PT,
	Size
};

/// <summary>
/// Main UI class.
/// </summary>
public ref class MainForm : public System::Windows::Forms::Form
{
public:
	/// <summary>
	/// UI Constructor.
	/// </summary>
	MainForm(void)
	{	
		InitializeComponent();
			
		//
		// Without Admin privileges, the tool will not be able to load the driver and enumerate all the processes.
		unsigned long err = Utils::InitAndStartDriver();
		if (err != 0)
		{
			if (err == /* ERROR_ACCESS_DENIED */ 5)
			{
				MessageBox::Show("Driver failed to load!\nTry running the tool as Administrator.");
			}
			else if (err == /* ERROR_INVALID_IMAGE_HASH */ 577)
			{
				MessageBox::Show("Driver failed to load!\nWindows needs to be running in the Test Signing Mode.\nbcdedit -set TESTSIGNING ON\nReboot computer.");
			}
			else
			{
				MessageBox::Show("Driver failed to load! Error:[" + err + "]");
			}
			stripStatusLabel->Text = "Driver failed to load! Error:[" + err + "]";
		}
		else if (!Utils::IsElevated())

		{
			MessageBox::Show("Try running the application as Administrator.\n"\
				"Without sufficient privileges it will not work properly!");
		}

		InitializeCustomComponents();

		//
		// Initialize singleton to access the form from outside of the class.
		if (m_instance == nullptr)
		{
			m_instance = this;
		}

		//
		// When starting the UI, populate the list of processes right away.
		EnumProcesses();
	}

public: System::Windows::Forms::DataGridView^ dgvProcesses;
public: System::Windows::Forms::DataGridView^ dgvMemory;
public: array< System::Windows::Forms::DataGridView^ >^ PTs = gcnew array< System::Windows::Forms::DataGridView^ >(4);
public: array< System::Windows::Forms::Label^ >^ decIndexes = gcnew array< System::Windows::Forms::Label^ >(5);
private: System::Windows::Forms::MenuStrip^ menuStrip1;
private: System::Windows::Forms::ToolStripMenuItem^ menu1ToolStripMenuItem;
private: System::Windows::Forms::ToolStripMenuItem^ aboutToolStripMenuItem;
private: System::Windows::Forms::ToolStripMenuItem^ aboutToolStripMenuItem1;
private: System::Windows::Forms::ToolStripMenuItem^ RefreshStripMenuItem;
private: System::Windows::Forms::ToolStripMenuItem^ exitToolStripMenuItem;
private: System::Windows::Forms::Panel^ panel1;
public: System::Windows::Forms::DataGridView^ dataGridViewPT;
public: System::Windows::Forms::DataGridView^ dataGridViewPDE;
public: System::Windows::Forms::DataGridView^ dataGridViewPDP;
public: System::Windows::Forms::DataGridView^ dataGridViewPML4;
private: System::Windows::Forms::RichTextBox^ richTextBoxMemory;
private: System::Windows::Forms::Label^ labelPhysicalAddress;
private: System::Windows::Forms::Label^ label15;
private: System::Windows::Forms::DataGridViewTextBoxColumn^ BaseAddress;
private: System::Windows::Forms::DataGridViewTextBoxColumn^ Protection;
private: System::Windows::Forms::Label^ labelVirtualAddressHex;
private: System::Windows::Forms::TextBox^ textBox29_21;
private: System::Windows::Forms::TextBox^ textBox20_12;
private: System::Windows::Forms::TextBox^ textBox38_30;
private: System::Windows::Forms::TextBox^ textBox47_39;
private: System::Windows::Forms::TextBox^ textBox11_00;
private: System::Windows::Forms::Button^ buttonUnpage;
private: System::Windows::Forms::CheckBox^ checkBoxAlwaysUnpage;
private: System::Windows::Forms::Label^ label17;
private: System::Windows::Forms::Label^ label13;
private: System::Windows::Forms::Label^ label16;
private: System::Windows::Forms::Label^ label11;
private: System::Windows::Forms::Label^ label12;
private: System::Windows::Forms::Label^ label9;
private: System::Windows::Forms::Label^ label10;
private: System::Windows::Forms::Label^ label8;
private: System::Windows::Forms::Label^ label6;
private: System::Windows::Forms::Label^ label5;
private: System::Windows::Forms::Label^ labelDec47_39;
private: System::Windows::Forms::Label^ labelDec38_30;
private: System::Windows::Forms::Label^ labelDec29_21;
private: System::Windows::Forms::Label^ labelDec20_12;
private: System::Windows::Forms::Label^ labelDec11_00;
private: System::Windows::Forms::Label^ labelPT;
private: System::Windows::Forms::Label^ labelPD;
private: System::Windows::Forms::Label^ labelPDP;
private: System::Windows::Forms::Label^ labelPML4;
private: System::Windows::Forms::Label^ labelBinaryAddress;
private: System::Windows::Forms::Label^ label1;
private: System::Windows::Forms::DataGridViewTextBoxColumn^ Index;
private: System::Windows::Forms::DataGridViewTextBoxColumn^ Value;
private: System::Windows::Forms::DataGridViewTextBoxColumn^ Index4;
private: System::Windows::Forms::DataGridViewTextBoxColumn^ Value4;
private: System::Windows::Forms::DataGridViewTextBoxColumn^ Index3;
private: System::Windows::Forms::DataGridViewTextBoxColumn^ Value3;
private: System::Windows::Forms::DataGridViewTextBoxColumn^ Index2;
private: System::Windows::Forms::DataGridViewTextBoxColumn^ Value2;
private: System::Windows::Forms::DataGridViewTextBoxColumn^ PID;
private: System::Windows::Forms::DataGridViewTextBoxColumn^ ProcessName;
private: System::Windows::Forms::TabPage^ AddressTab;
private: System::Windows::Forms::ListBox^ addressListBox;
private: System::Windows::Forms::TabPage^ RegistersTab;
public: System::Windows::Forms::DataGridView^ dataGridCR3;
private: System::Windows::Forms::DataGridViewTextBoxColumn^ dataGridViewTextBoxColumn1;
private: System::Windows::Forms::DataGridViewTextBoxColumn^ Column17;
private: System::Windows::Forms::DataGridViewTextBoxColumn^ dataGridViewTextBoxColumn2;
private: System::Windows::Forms::DataGridViewTextBoxColumn^ dataGridViewTextBoxColumn3;
private: System::Windows::Forms::DataGridViewTextBoxColumn^ dataGridViewTextBoxColumn4;
private: System::Windows::Forms::DataGridViewTextBoxColumn^ dataGridViewTextBoxColumn5;
private: System::Windows::Forms::DataGridViewTextBoxColumn^ dataGridViewTextBoxColumn6;
public: System::Windows::Forms::Label^ labelCR2;
public: System::Windows::Forms::DataGridView^ dataGridCR0;
private: System::Windows::Forms::DataGridViewTextBoxColumn^ dataGridViewTextBoxColumn7;
private: System::Windows::Forms::DataGridViewTextBoxColumn^ Column16;
private: System::Windows::Forms::DataGridViewTextBoxColumn^ Column15;
private: System::Windows::Forms::DataGridViewTextBoxColumn^ dataGridViewTextBoxColumn8;
private: System::Windows::Forms::DataGridViewTextBoxColumn^ dataGridViewTextBoxColumn9;
private: System::Windows::Forms::DataGridViewTextBoxColumn^ dataGridViewTextBoxColumn10;
private: System::Windows::Forms::DataGridViewTextBoxColumn^ dataGridViewTextBoxColumn11;
private: System::Windows::Forms::DataGridViewTextBoxColumn^ dataGridViewTextBoxColumn12;
private: System::Windows::Forms::DataGridViewTextBoxColumn^ dataGridViewTextBoxColumn13;
private: System::Windows::Forms::DataGridViewTextBoxColumn^ dataGridViewTextBoxColumn14;
private: System::Windows::Forms::DataGridViewTextBoxColumn^ dataGridViewTextBoxColumn15;
private: System::Windows::Forms::DataGridViewTextBoxColumn^ dataGridViewTextBoxColumn16;
private: System::Windows::Forms::DataGridViewTextBoxColumn^ dataGridViewTextBoxColumn17;
private: System::Windows::Forms::DataGridViewTextBoxColumn^ dataGridViewTextBoxColumn18;
private: System::Windows::Forms::DataGridViewTextBoxColumn^ dataGridViewTextBoxColumn19;
private: System::Windows::Forms::DataGridViewTextBoxColumn^ dataGridViewTextBoxColumn20;
private: System::Windows::Forms::TabPage^ TablesTab;
public: System::Windows::Forms::DataGridView^ dgvPTProperties;
private: System::Windows::Forms::DataGridViewTextBoxColumn^ dataGridViewTextBoxColumn21;
private: System::Windows::Forms::DataGridViewTextBoxColumn^ dataGridViewTextBoxColumn22;
private: System::Windows::Forms::DataGridViewTextBoxColumn^ dataGridViewTextBoxColumn23;
private: System::Windows::Forms::DataGridViewTextBoxColumn^ Column19;
private: System::Windows::Forms::DataGridViewTextBoxColumn^ dataGridViewTextBoxColumn24;
private: System::Windows::Forms::DataGridViewTextBoxColumn^ dataGridViewTextBoxColumn25;
private: System::Windows::Forms::DataGridViewTextBoxColumn^ dataGridViewTextBoxColumn26;
private: System::Windows::Forms::DataGridViewTextBoxColumn^ Column18;
private: System::Windows::Forms::DataGridViewTextBoxColumn^ dataGridViewTextBoxColumn27;
private: System::Windows::Forms::DataGridViewTextBoxColumn^ dataGridViewTextBoxColumn28;
private: System::Windows::Forms::DataGridViewTextBoxColumn^ dataGridViewTextBoxColumn29;
private: System::Windows::Forms::DataGridViewTextBoxColumn^ dataGridViewTextBoxColumn30;
private: System::Windows::Forms::DataGridViewTextBoxColumn^ dataGridViewTextBoxColumn31;
private: System::Windows::Forms::DataGridViewTextBoxColumn^ dataGridViewTextBoxColumn32;
private: System::Windows::Forms::DataGridViewTextBoxColumn^ dataGridViewTextBoxColumn33;
private: System::Windows::Forms::DataGridViewTextBoxColumn^ dataGridViewTextBoxColumn34;
public: System::Windows::Forms::DataGridView^ dgvProperties;
private: System::Windows::Forms::DataGridViewTextBoxColumn^ Column14;
private: System::Windows::Forms::DataGridViewTextBoxColumn^ Column1;
private: System::Windows::Forms::DataGridViewTextBoxColumn^ Column2;
private: System::Windows::Forms::DataGridViewTextBoxColumn^ Column3;
private: System::Windows::Forms::DataGridViewTextBoxColumn^ Column4;
private: System::Windows::Forms::DataGridViewTextBoxColumn^ Column5;
private: System::Windows::Forms::DataGridViewTextBoxColumn^ Column6;
private: System::Windows::Forms::DataGridViewTextBoxColumn^ Column7;
private: System::Windows::Forms::DataGridViewTextBoxColumn^ Column8;
private: System::Windows::Forms::DataGridViewTextBoxColumn^ Column9;
private: System::Windows::Forms::DataGridViewTextBoxColumn^ Column10;
private: System::Windows::Forms::DataGridViewTextBoxColumn^ Column11;
private: System::Windows::Forms::DataGridViewTextBoxColumn^ Column12;
private: System::Windows::Forms::DataGridViewTextBoxColumn^ Column13;
private: System::Windows::Forms::TabControl^ PropertiesTabs;
private: static MainForm^ m_instance{ nullptr };
protected:
	/// <summary>
	/// Clean up any resources being used.
	/// </summary>
	~MainForm()
	{
		Utils::StopAndDeleteDriver();
		if (components)
		{
			delete components;
		}
	}
public: static MainForm^ GetInstance()
{
	return m_instance;
}
private: System::Windows::Forms::Button^ Refresh_button;
private: System::Windows::Forms::StatusStrip^ statusStrip;
public: System::Windows::Forms::ToolStripStatusLabel^ stripStatusLabel;
private:
	/// <summary>
	/// Required designer variable.
	/// </summary>
	System::ComponentModel::Container ^components;

#pragma region Windows Form Designer generated code
	/// <summary>
	/// Required method for Designer support - do not modify
	/// the contents of this method with the code editor.
	/// </summary>
	void InitializeComponent(void)
	{
		System::Windows::Forms::DataGridViewCellStyle^ dataGridViewCellStyle1 = (gcnew System::Windows::Forms::DataGridViewCellStyle());
		System::Windows::Forms::DataGridViewCellStyle^ dataGridViewCellStyle2 = (gcnew System::Windows::Forms::DataGridViewCellStyle());
		System::Windows::Forms::DataGridViewCellStyle^ dataGridViewCellStyle3 = (gcnew System::Windows::Forms::DataGridViewCellStyle());
		System::Windows::Forms::DataGridViewCellStyle^ dataGridViewCellStyle4 = (gcnew System::Windows::Forms::DataGridViewCellStyle());
		System::Windows::Forms::DataGridViewCellStyle^ dataGridViewCellStyle5 = (gcnew System::Windows::Forms::DataGridViewCellStyle());
		System::Windows::Forms::DataGridViewCellStyle^ dataGridViewCellStyle6 = (gcnew System::Windows::Forms::DataGridViewCellStyle());
		System::Windows::Forms::DataGridViewCellStyle^ dataGridViewCellStyle7 = (gcnew System::Windows::Forms::DataGridViewCellStyle());
		System::Windows::Forms::DataGridViewCellStyle^ dataGridViewCellStyle8 = (gcnew System::Windows::Forms::DataGridViewCellStyle());
		System::Windows::Forms::DataGridViewCellStyle^ dataGridViewCellStyle9 = (gcnew System::Windows::Forms::DataGridViewCellStyle());
		System::Windows::Forms::DataGridViewCellStyle^ dataGridViewCellStyle10 = (gcnew System::Windows::Forms::DataGridViewCellStyle());
		System::Windows::Forms::DataGridViewCellStyle^ dataGridViewCellStyle11 = (gcnew System::Windows::Forms::DataGridViewCellStyle());
		System::Windows::Forms::DataGridViewCellStyle^ dataGridViewCellStyle12 = (gcnew System::Windows::Forms::DataGridViewCellStyle());
		System::Windows::Forms::DataGridViewCellStyle^ dataGridViewCellStyle13 = (gcnew System::Windows::Forms::DataGridViewCellStyle());
		System::Windows::Forms::DataGridViewCellStyle^ dataGridViewCellStyle14 = (gcnew System::Windows::Forms::DataGridViewCellStyle());
		System::Windows::Forms::DataGridViewCellStyle^ dataGridViewCellStyle15 = (gcnew System::Windows::Forms::DataGridViewCellStyle());
		System::Windows::Forms::DataGridViewCellStyle^ dataGridViewCellStyle16 = (gcnew System::Windows::Forms::DataGridViewCellStyle());
		System::Windows::Forms::DataGridViewCellStyle^ dataGridViewCellStyle17 = (gcnew System::Windows::Forms::DataGridViewCellStyle());
		System::Windows::Forms::DataGridViewCellStyle^ dataGridViewCellStyle18 = (gcnew System::Windows::Forms::DataGridViewCellStyle());
		System::Windows::Forms::DataGridViewCellStyle^ dataGridViewCellStyle19 = (gcnew System::Windows::Forms::DataGridViewCellStyle());
		System::Windows::Forms::DataGridViewCellStyle^ dataGridViewCellStyle20 = (gcnew System::Windows::Forms::DataGridViewCellStyle());
		System::Windows::Forms::DataGridViewCellStyle^ dataGridViewCellStyle21 = (gcnew System::Windows::Forms::DataGridViewCellStyle());
		System::Windows::Forms::DataGridViewCellStyle^ dataGridViewCellStyle22 = (gcnew System::Windows::Forms::DataGridViewCellStyle());
		System::Windows::Forms::DataGridViewCellStyle^ dataGridViewCellStyle23 = (gcnew System::Windows::Forms::DataGridViewCellStyle());
		System::Windows::Forms::DataGridViewCellStyle^ dataGridViewCellStyle24 = (gcnew System::Windows::Forms::DataGridViewCellStyle());
		System::Windows::Forms::DataGridViewCellStyle^ dataGridViewCellStyle25 = (gcnew System::Windows::Forms::DataGridViewCellStyle());
		System::Windows::Forms::DataGridViewCellStyle^ dataGridViewCellStyle26 = (gcnew System::Windows::Forms::DataGridViewCellStyle());
		System::Windows::Forms::DataGridViewCellStyle^ dataGridViewCellStyle27 = (gcnew System::Windows::Forms::DataGridViewCellStyle());
		System::Windows::Forms::DataGridViewCellStyle^ dataGridViewCellStyle28 = (gcnew System::Windows::Forms::DataGridViewCellStyle());
		System::Windows::Forms::DataGridViewCellStyle^ dataGridViewCellStyle29 = (gcnew System::Windows::Forms::DataGridViewCellStyle());
		System::Windows::Forms::DataGridViewCellStyle^ dataGridViewCellStyle30 = (gcnew System::Windows::Forms::DataGridViewCellStyle());
		System::Windows::Forms::DataGridViewCellStyle^ dataGridViewCellStyle31 = (gcnew System::Windows::Forms::DataGridViewCellStyle());
		System::Windows::Forms::DataGridViewCellStyle^ dataGridViewCellStyle32 = (gcnew System::Windows::Forms::DataGridViewCellStyle());
		System::Windows::Forms::DataGridViewCellStyle^ dataGridViewCellStyle33 = (gcnew System::Windows::Forms::DataGridViewCellStyle());
		System::Windows::Forms::DataGridViewCellStyle^ dataGridViewCellStyle34 = (gcnew System::Windows::Forms::DataGridViewCellStyle());
		System::Windows::Forms::DataGridViewCellStyle^ dataGridViewCellStyle35 = (gcnew System::Windows::Forms::DataGridViewCellStyle());
		System::Windows::Forms::DataGridViewCellStyle^ dataGridViewCellStyle36 = (gcnew System::Windows::Forms::DataGridViewCellStyle());
		System::Windows::Forms::DataGridViewCellStyle^ dataGridViewCellStyle37 = (gcnew System::Windows::Forms::DataGridViewCellStyle());
		System::Windows::Forms::DataGridViewCellStyle^ dataGridViewCellStyle38 = (gcnew System::Windows::Forms::DataGridViewCellStyle());
		System::Windows::Forms::DataGridViewCellStyle^ dataGridViewCellStyle39 = (gcnew System::Windows::Forms::DataGridViewCellStyle());
		System::Windows::Forms::DataGridViewCellStyle^ dataGridViewCellStyle40 = (gcnew System::Windows::Forms::DataGridViewCellStyle());
		System::Windows::Forms::DataGridViewCellStyle^ dataGridViewCellStyle41 = (gcnew System::Windows::Forms::DataGridViewCellStyle());
		System::Windows::Forms::DataGridViewCellStyle^ dataGridViewCellStyle42 = (gcnew System::Windows::Forms::DataGridViewCellStyle());
		System::Windows::Forms::DataGridViewCellStyle^ dataGridViewCellStyle43 = (gcnew System::Windows::Forms::DataGridViewCellStyle());
		System::Windows::Forms::DataGridViewCellStyle^ dataGridViewCellStyle44 = (gcnew System::Windows::Forms::DataGridViewCellStyle());
		System::Windows::Forms::DataGridViewCellStyle^ dataGridViewCellStyle45 = (gcnew System::Windows::Forms::DataGridViewCellStyle());
		System::Windows::Forms::DataGridViewCellStyle^ dataGridViewCellStyle46 = (gcnew System::Windows::Forms::DataGridViewCellStyle());
		System::Windows::Forms::DataGridViewCellStyle^ dataGridViewCellStyle47 = (gcnew System::Windows::Forms::DataGridViewCellStyle());
		System::Windows::Forms::DataGridViewCellStyle^ dataGridViewCellStyle48 = (gcnew System::Windows::Forms::DataGridViewCellStyle());
		System::Windows::Forms::DataGridViewCellStyle^ dataGridViewCellStyle49 = (gcnew System::Windows::Forms::DataGridViewCellStyle());
		System::Windows::Forms::DataGridViewCellStyle^ dataGridViewCellStyle50 = (gcnew System::Windows::Forms::DataGridViewCellStyle());
		System::Windows::Forms::DataGridViewCellStyle^ dataGridViewCellStyle51 = (gcnew System::Windows::Forms::DataGridViewCellStyle());
		System::Windows::Forms::DataGridViewCellStyle^ dataGridViewCellStyle52 = (gcnew System::Windows::Forms::DataGridViewCellStyle());
		System::Windows::Forms::DataGridViewCellStyle^ dataGridViewCellStyle53 = (gcnew System::Windows::Forms::DataGridViewCellStyle());
		System::Windows::Forms::DataGridViewCellStyle^ dataGridViewCellStyle54 = (gcnew System::Windows::Forms::DataGridViewCellStyle());
		System::Windows::Forms::DataGridViewCellStyle^ dataGridViewCellStyle55 = (gcnew System::Windows::Forms::DataGridViewCellStyle());
		this->panel1 = (gcnew System::Windows::Forms::Panel());
		this->labelBinaryAddress = (gcnew System::Windows::Forms::Label());
		this->labelPT = (gcnew System::Windows::Forms::Label());
		this->labelPD = (gcnew System::Windows::Forms::Label());
		this->labelPDP = (gcnew System::Windows::Forms::Label());
		this->labelPML4 = (gcnew System::Windows::Forms::Label());
		this->labelDec47_39 = (gcnew System::Windows::Forms::Label());
		this->labelDec38_30 = (gcnew System::Windows::Forms::Label());
		this->labelDec29_21 = (gcnew System::Windows::Forms::Label());
		this->labelDec20_12 = (gcnew System::Windows::Forms::Label());
		this->labelDec11_00 = (gcnew System::Windows::Forms::Label());
		this->label17 = (gcnew System::Windows::Forms::Label());
		this->label13 = (gcnew System::Windows::Forms::Label());
		this->label16 = (gcnew System::Windows::Forms::Label());
		this->label11 = (gcnew System::Windows::Forms::Label());
		this->label12 = (gcnew System::Windows::Forms::Label());
		this->label9 = (gcnew System::Windows::Forms::Label());
		this->label10 = (gcnew System::Windows::Forms::Label());
		this->label8 = (gcnew System::Windows::Forms::Label());
		this->label6 = (gcnew System::Windows::Forms::Label());
		this->label5 = (gcnew System::Windows::Forms::Label());
		this->checkBoxAlwaysUnpage = (gcnew System::Windows::Forms::CheckBox());
		this->buttonUnpage = (gcnew System::Windows::Forms::Button());
		this->textBox11_00 = (gcnew System::Windows::Forms::TextBox());
		this->textBox29_21 = (gcnew System::Windows::Forms::TextBox());
		this->textBox20_12 = (gcnew System::Windows::Forms::TextBox());
		this->textBox38_30 = (gcnew System::Windows::Forms::TextBox());
		this->textBox47_39 = (gcnew System::Windows::Forms::TextBox());
		this->labelVirtualAddressHex = (gcnew System::Windows::Forms::Label());
		this->label15 = (gcnew System::Windows::Forms::Label());
		this->labelPhysicalAddress = (gcnew System::Windows::Forms::Label());
		this->richTextBoxMemory = (gcnew System::Windows::Forms::RichTextBox());
		this->dataGridViewPT = (gcnew System::Windows::Forms::DataGridView());
		this->Index4 = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
		this->Value4 = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
		this->dataGridViewPDE = (gcnew System::Windows::Forms::DataGridView());
		this->Index3 = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
		this->Value3 = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
		this->dataGridViewPDP = (gcnew System::Windows::Forms::DataGridView());
		this->Index2 = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
		this->Value2 = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
		this->dataGridViewPML4 = (gcnew System::Windows::Forms::DataGridView());
		this->Index = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
		this->Value = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
		this->Refresh_button = (gcnew System::Windows::Forms::Button());
		this->statusStrip = (gcnew System::Windows::Forms::StatusStrip());
		this->stripStatusLabel = (gcnew System::Windows::Forms::ToolStripStatusLabel());
		this->dgvProcesses = (gcnew System::Windows::Forms::DataGridView());
		this->PID = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
		this->ProcessName = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
		this->dgvMemory = (gcnew System::Windows::Forms::DataGridView());
		this->BaseAddress = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
		this->Protection = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
		this->menuStrip1 = (gcnew System::Windows::Forms::MenuStrip());
		this->menu1ToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
		this->RefreshStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
		this->exitToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
		this->aboutToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
		this->aboutToolStripMenuItem1 = (gcnew System::Windows::Forms::ToolStripMenuItem());
		this->label1 = (gcnew System::Windows::Forms::Label());
		this->AddressTab = (gcnew System::Windows::Forms::TabPage());
		this->addressListBox = (gcnew System::Windows::Forms::ListBox());
		this->RegistersTab = (gcnew System::Windows::Forms::TabPage());
		this->dataGridCR3 = (gcnew System::Windows::Forms::DataGridView());
		this->dataGridViewTextBoxColumn1 = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
		this->Column17 = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
		this->dataGridViewTextBoxColumn2 = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
		this->dataGridViewTextBoxColumn3 = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
		this->dataGridViewTextBoxColumn4 = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
		this->dataGridViewTextBoxColumn5 = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
		this->dataGridViewTextBoxColumn6 = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
		this->labelCR2 = (gcnew System::Windows::Forms::Label());
		this->dataGridCR0 = (gcnew System::Windows::Forms::DataGridView());
		this->dataGridViewTextBoxColumn7 = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
		this->Column16 = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
		this->Column15 = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
		this->dataGridViewTextBoxColumn8 = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
		this->dataGridViewTextBoxColumn9 = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
		this->dataGridViewTextBoxColumn10 = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
		this->dataGridViewTextBoxColumn11 = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
		this->dataGridViewTextBoxColumn12 = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
		this->dataGridViewTextBoxColumn13 = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
		this->dataGridViewTextBoxColumn14 = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
		this->dataGridViewTextBoxColumn15 = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
		this->dataGridViewTextBoxColumn16 = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
		this->dataGridViewTextBoxColumn17 = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
		this->dataGridViewTextBoxColumn18 = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
		this->dataGridViewTextBoxColumn19 = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
		this->dataGridViewTextBoxColumn20 = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
		this->TablesTab = (gcnew System::Windows::Forms::TabPage());
		this->dgvPTProperties = (gcnew System::Windows::Forms::DataGridView());
		this->dataGridViewTextBoxColumn21 = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
		this->dataGridViewTextBoxColumn22 = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
		this->dataGridViewTextBoxColumn23 = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
		this->Column19 = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
		this->dataGridViewTextBoxColumn24 = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
		this->dataGridViewTextBoxColumn25 = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
		this->dataGridViewTextBoxColumn26 = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
		this->Column18 = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
		this->dataGridViewTextBoxColumn27 = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
		this->dataGridViewTextBoxColumn28 = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
		this->dataGridViewTextBoxColumn29 = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
		this->dataGridViewTextBoxColumn30 = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
		this->dataGridViewTextBoxColumn31 = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
		this->dataGridViewTextBoxColumn32 = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
		this->dataGridViewTextBoxColumn33 = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
		this->dataGridViewTextBoxColumn34 = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
		this->dgvProperties = (gcnew System::Windows::Forms::DataGridView());
		this->Column14 = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
		this->Column1 = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
		this->Column2 = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
		this->Column3 = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
		this->Column4 = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
		this->Column5 = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
		this->Column6 = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
		this->Column7 = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
		this->Column8 = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
		this->Column9 = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
		this->Column10 = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
		this->Column11 = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
		this->Column12 = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
		this->Column13 = (gcnew System::Windows::Forms::DataGridViewTextBoxColumn());
		this->PropertiesTabs = (gcnew System::Windows::Forms::TabControl());
		this->panel1->SuspendLayout();
		(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->dataGridViewPT))->BeginInit();
		(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->dataGridViewPDE))->BeginInit();
		(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->dataGridViewPDP))->BeginInit();
		(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->dataGridViewPML4))->BeginInit();
		this->statusStrip->SuspendLayout();
		(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->dgvProcesses))->BeginInit();
		(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->dgvMemory))->BeginInit();
		this->menuStrip1->SuspendLayout();
		this->AddressTab->SuspendLayout();
		this->RegistersTab->SuspendLayout();
		(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->dataGridCR3))->BeginInit();
		(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->dataGridCR0))->BeginInit();
		this->TablesTab->SuspendLayout();
		(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->dgvPTProperties))->BeginInit();
		(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->dgvProperties))->BeginInit();
		this->PropertiesTabs->SuspendLayout();
		this->SuspendLayout();
		// 
		// panel1
		// 
		this->panel1->BorderStyle = System::Windows::Forms::BorderStyle::FixedSingle;
		this->panel1->Controls->Add(this->labelBinaryAddress);
		this->panel1->Controls->Add(this->labelPT);
		this->panel1->Controls->Add(this->labelPD);
		this->panel1->Controls->Add(this->labelPDP);
		this->panel1->Controls->Add(this->labelPML4);
		this->panel1->Controls->Add(this->labelDec47_39);
		this->panel1->Controls->Add(this->labelDec38_30);
		this->panel1->Controls->Add(this->labelDec29_21);
		this->panel1->Controls->Add(this->labelDec20_12);
		this->panel1->Controls->Add(this->labelDec11_00);
		this->panel1->Controls->Add(this->label17);
		this->panel1->Controls->Add(this->label13);
		this->panel1->Controls->Add(this->label16);
		this->panel1->Controls->Add(this->label11);
		this->panel1->Controls->Add(this->label12);
		this->panel1->Controls->Add(this->label9);
		this->panel1->Controls->Add(this->label10);
		this->panel1->Controls->Add(this->label8);
		this->panel1->Controls->Add(this->label6);
		this->panel1->Controls->Add(this->label5);
		this->panel1->Controls->Add(this->checkBoxAlwaysUnpage);
		this->panel1->Controls->Add(this->buttonUnpage);
		this->panel1->Controls->Add(this->textBox11_00);
		this->panel1->Controls->Add(this->textBox29_21);
		this->panel1->Controls->Add(this->textBox20_12);
		this->panel1->Controls->Add(this->textBox38_30);
		this->panel1->Controls->Add(this->textBox47_39);
		this->panel1->Controls->Add(this->labelVirtualAddressHex);
		this->panel1->Controls->Add(this->label15);
		this->panel1->Controls->Add(this->labelPhysicalAddress);
		this->panel1->Controls->Add(this->richTextBoxMemory);
		this->panel1->Controls->Add(this->dataGridViewPT);
		this->panel1->Controls->Add(this->dataGridViewPDE);
		this->panel1->Controls->Add(this->dataGridViewPDP);
		this->panel1->Controls->Add(this->dataGridViewPML4);
		this->panel1->Location = System::Drawing::Point(515, 27);
		this->panel1->Name = L"panel1";
		this->panel1->Size = System::Drawing::Size(840, 676);
		this->panel1->TabIndex = 30;
		// 
		// labelBinaryAddress
		// 
		this->labelBinaryAddress->AutoSize = true;
		this->labelBinaryAddress->Location = System::Drawing::Point(177, 9);
		this->labelBinaryAddress->Name = L"labelBinaryAddress";
		this->labelBinaryAddress->Size = System::Drawing::Size(76, 13);
		this->labelBinaryAddress->TabIndex = 72;
		this->labelBinaryAddress->Text = L"Binery address";
		// 
		// labelPT
		// 
		this->labelPT->AutoSize = true;
		this->labelPT->Location = System::Drawing::Point(642, 139);
		this->labelPT->Name = L"labelPT";
		this->labelPT->Size = System::Drawing::Size(62, 13);
		this->labelPT->TabIndex = 71;
		this->labelPT->Text = L"Page Table";
		// 
		// labelPD
		// 
		this->labelPD->AutoSize = true;
		this->labelPD->Location = System::Drawing::Point(443, 139);
		this->labelPD->Name = L"labelPD";
		this->labelPD->Size = System::Drawing::Size(77, 13);
		this->labelPD->TabIndex = 70;
		this->labelPD->Text = L"Page Directory";
		// 
		// labelPDP
		// 
		this->labelPDP->AutoSize = true;
		this->labelPDP->Location = System::Drawing::Point(243, 139);
		this->labelPDP->Name = L"labelPDP";
		this->labelPDP->Size = System::Drawing::Size(113, 13);
		this->labelPDP->TabIndex = 69;
		this->labelPDP->Text = L"Page Directory Pointer";
		// 
		// labelPML4
		// 
		this->labelPML4->AutoSize = true;
		this->labelPML4->Location = System::Drawing::Point(44, 139);
		this->labelPML4->Name = L"labelPML4";
		this->labelPML4->Size = System::Drawing::Size(94, 13);
		this->labelPML4->TabIndex = 68;
		this->labelPML4->Text = L"Page Map Level 4";
		// 
		// labelDec47_39
		// 
		this->labelDec47_39->AutoSize = true;
		this->labelDec47_39->Location = System::Drawing::Point(178, 73);
		this->labelDec47_39->Name = L"labelDec47_39";
		this->labelDec47_39->Size = System::Drawing::Size(13, 13);
		this->labelDec47_39->TabIndex = 67;
		this->labelDec47_39->Text = L"0";
		this->labelDec47_39->TextAlign = System::Drawing::ContentAlignment::MiddleCenter;
		// 
		// labelDec38_30
		// 
		this->labelDec38_30->AutoSize = true;
		this->labelDec38_30->Location = System::Drawing::Point(278, 73);
		this->labelDec38_30->Name = L"labelDec38_30";
		this->labelDec38_30->Size = System::Drawing::Size(13, 13);
		this->labelDec38_30->TabIndex = 65;
		this->labelDec38_30->Text = L"0";
		this->labelDec38_30->TextAlign = System::Drawing::ContentAlignment::MiddleCenter;
		// 
		// labelDec29_21
		// 
		this->labelDec29_21->AutoSize = true;
		this->labelDec29_21->Location = System::Drawing::Point(378, 73);
		this->labelDec29_21->Name = L"labelDec29_21";
		this->labelDec29_21->Size = System::Drawing::Size(13, 13);
		this->labelDec29_21->TabIndex = 64;
		this->labelDec29_21->Text = L"0";
		this->labelDec29_21->TextAlign = System::Drawing::ContentAlignment::MiddleCenter;
		// 
		// labelDec20_12
		// 
		this->labelDec20_12->AutoSize = true;
		this->labelDec20_12->Location = System::Drawing::Point(478, 73);
		this->labelDec20_12->Name = L"labelDec20_12";
		this->labelDec20_12->Size = System::Drawing::Size(13, 13);
		this->labelDec20_12->TabIndex = 63;
		this->labelDec20_12->Text = L"0";
		this->labelDec20_12->TextAlign = System::Drawing::ContentAlignment::MiddleCenter;
		// 
		// labelDec11_00
		// 
		this->labelDec11_00->AutoSize = true;
		this->labelDec11_00->Location = System::Drawing::Point(578, 73);
		this->labelDec11_00->Name = L"labelDec11_00";
		this->labelDec11_00->Size = System::Drawing::Size(13, 13);
		this->labelDec11_00->TabIndex = 62;
		this->labelDec11_00->Text = L"0";
		this->labelDec11_00->TextAlign = System::Drawing::ContentAlignment::MiddleCenter;
		// 
		// label17
		// 
		this->label17->AutoSize = true;
		this->label17->Location = System::Drawing::Point(134, 34);
		this->label17->Name = L"label17";
		this->label17->Size = System::Drawing::Size(19, 13);
		this->label17->TabIndex = 61;
		this->label17->Text = L"47";
		// 
		// label13
		// 
		this->label13->AutoSize = true;
		this->label13->Location = System::Drawing::Point(218, 34);
		this->label13->Name = L"label13";
		this->label13->Size = System::Drawing::Size(19, 13);
		this->label13->TabIndex = 60;
		this->label13->Text = L"39";
		// 
		// label16
		// 
		this->label16->AutoSize = true;
		this->label16->Location = System::Drawing::Point(234, 34);
		this->label16->Name = L"label16";
		this->label16->Size = System::Drawing::Size(19, 13);
		this->label16->TabIndex = 59;
		this->label16->Text = L"38";
		// 
		// label11
		// 
		this->label11->AutoSize = true;
		this->label11->Location = System::Drawing::Point(318, 34);
		this->label11->Name = L"label11";
		this->label11->Size = System::Drawing::Size(19, 13);
		this->label11->TabIndex = 58;
		this->label11->Text = L"30";
		// 
		// label12
		// 
		this->label12->AutoSize = true;
		this->label12->Location = System::Drawing::Point(334, 34);
		this->label12->Name = L"label12";
		this->label12->Size = System::Drawing::Size(19, 13);
		this->label12->TabIndex = 57;
		this->label12->Text = L"29";
		// 
		// label9
		// 
		this->label9->AutoSize = true;
		this->label9->Location = System::Drawing::Point(418, 34);
		this->label9->Name = L"label9";
		this->label9->Size = System::Drawing::Size(19, 13);
		this->label9->TabIndex = 56;
		this->label9->Text = L"21";
		// 
		// label10
		// 
		this->label10->AutoSize = true;
		this->label10->Location = System::Drawing::Point(434, 34);
		this->label10->Name = L"label10";
		this->label10->Size = System::Drawing::Size(19, 13);
		this->label10->TabIndex = 55;
		this->label10->Text = L"20";
		// 
		// label8
		// 
		this->label8->AutoSize = true;
		this->label8->Location = System::Drawing::Point(518, 34);
		this->label8->Name = L"label8";
		this->label8->Size = System::Drawing::Size(19, 13);
		this->label8->TabIndex = 54;
		this->label8->Text = L"12";
		// 
		// label6
		// 
		this->label6->AutoSize = true;
		this->label6->Location = System::Drawing::Point(534, 34);
		this->label6->Name = L"label6";
		this->label6->Size = System::Drawing::Size(19, 13);
		this->label6->TabIndex = 53;
		this->label6->Text = L"11";
		// 
		// label5
		// 
		this->label5->AutoSize = true;
		this->label5->Location = System::Drawing::Point(624, 34);
		this->label5->Name = L"label5";
		this->label5->Size = System::Drawing::Size(13, 13);
		this->label5->TabIndex = 52;
		this->label5->Text = L"0";
		// 
		// checkBoxAlwaysUnpage
		// 
		this->checkBoxAlwaysUnpage->AutoSize = true;
		this->checkBoxAlwaysUnpage->Location = System::Drawing::Point(697, 466);
		this->checkBoxAlwaysUnpage->Name = L"checkBoxAlwaysUnpage";
		this->checkBoxAlwaysUnpage->Size = System::Drawing::Size(98, 17);
		this->checkBoxAlwaysUnpage->TabIndex = 51;
		this->checkBoxAlwaysUnpage->Text = L"Always unpage";
		this->checkBoxAlwaysUnpage->UseVisualStyleBackColor = true;
		// 
		// buttonUnpage
		// 
		this->buttonUnpage->Location = System::Drawing::Point(720, 489);
		this->buttonUnpage->Name = L"buttonUnpage";
		this->buttonUnpage->Size = System::Drawing::Size(75, 23);
		this->buttonUnpage->TabIndex = 50;
		this->buttonUnpage->Text = L"Page In";
		this->buttonUnpage->UseVisualStyleBackColor = true;
		this->buttonUnpage->Click += gcnew System::EventHandler(this, &MainForm::ButtonUnpage_Click);
		// 
		// textBox11_00
		// 
		this->textBox11_00->BorderStyle = System::Windows::Forms::BorderStyle::FixedSingle;
		this->textBox11_00->Location = System::Drawing::Point(533, 50);
		this->textBox11_00->Name = L"textBox11_00";
		this->textBox11_00->ReadOnly = true;
		this->textBox11_00->Size = System::Drawing::Size(100, 20);
		this->textBox11_00->TabIndex = 49;
		this->textBox11_00->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
		// 
		// textBox29_21
		// 
		this->textBox29_21->BorderStyle = System::Windows::Forms::BorderStyle::FixedSingle;
		this->textBox29_21->Location = System::Drawing::Point(335, 50);
		this->textBox29_21->Name = L"textBox29_21";
		this->textBox29_21->ReadOnly = true;
		this->textBox29_21->Size = System::Drawing::Size(100, 20);
		this->textBox29_21->TabIndex = 48;
		this->textBox29_21->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
		// 
		// textBox20_12
		// 
		this->textBox20_12->BorderStyle = System::Windows::Forms::BorderStyle::FixedSingle;
		this->textBox20_12->Location = System::Drawing::Point(434, 50);
		this->textBox20_12->Name = L"textBox20_12";
		this->textBox20_12->ReadOnly = true;
		this->textBox20_12->Size = System::Drawing::Size(100, 20);
		this->textBox20_12->TabIndex = 47;
		this->textBox20_12->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
		// 
		// textBox38_30
		// 
		this->textBox38_30->BorderStyle = System::Windows::Forms::BorderStyle::FixedSingle;
		this->textBox38_30->Location = System::Drawing::Point(236, 50);
		this->textBox38_30->Name = L"textBox38_30";
		this->textBox38_30->ReadOnly = true;
		this->textBox38_30->Size = System::Drawing::Size(100, 20);
		this->textBox38_30->TabIndex = 46;
		this->textBox38_30->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
		// 
		// textBox47_39
		// 
		this->textBox47_39->BorderStyle = System::Windows::Forms::BorderStyle::FixedSingle;
		this->textBox47_39->Location = System::Drawing::Point(137, 50);
		this->textBox47_39->Name = L"textBox47_39";
		this->textBox47_39->ReadOnly = true;
		this->textBox47_39->Size = System::Drawing::Size(100, 20);
		this->textBox47_39->TabIndex = 45;
		this->textBox47_39->TextAlign = System::Windows::Forms::HorizontalAlignment::Center;
		// 
		// labelVirtualAddressHex
		// 
		this->labelVirtualAddressHex->AutoSize = true;
		this->labelVirtualAddressHex->Location = System::Drawing::Point(6, 9);
		this->labelVirtualAddressHex->Name = L"labelVirtualAddressHex";
		this->labelVirtualAddressHex->Size = System::Drawing::Size(77, 13);
		this->labelVirtualAddressHex->TabIndex = 44;
		this->labelVirtualAddressHex->Text = L"Virtual Address";
		// 
		// label15
		// 
		this->label15->AutoSize = true;
		this->label15->Location = System::Drawing::Point(6, 52);
		this->label15->Name = L"label15";
		this->label15->Size = System::Drawing::Size(28, 13);
		this->label15->TabIndex = 43;
		this->label15->Text = L"CR3";
		// 
		// labelPhysicalAddress
		// 
		this->labelPhysicalAddress->AutoSize = true;
		this->labelPhysicalAddress->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 8.25F, System::Drawing::FontStyle::Bold,
			System::Drawing::GraphicsUnit::Point, static_cast<System::Byte>(204)));
		this->labelPhysicalAddress->Location = System::Drawing::Point(578, 9);
		this->labelPhysicalAddress->Name = L"labelPhysicalAddress";
		this->labelPhysicalAddress->Size = System::Drawing::Size(102, 13);
		this->labelPhysicalAddress->TabIndex = 33;
		this->labelPhysicalAddress->Text = L"Physical address";
		// 
		// richTextBoxMemory
		// 
		this->richTextBoxMemory->Font = (gcnew System::Drawing::Font(L"Cascadia Code", 8.25F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point,
			static_cast<System::Byte>(204)));
		this->richTextBoxMemory->Location = System::Drawing::Point(47, 463);
		this->richTextBoxMemory->Name = L"richTextBoxMemory";
		this->richTextBoxMemory->ReadOnly = true;
		this->richTextBoxMemory->Size = System::Drawing::Size(633, 208);
		this->richTextBoxMemory->TabIndex = 41;
		this->richTextBoxMemory->Text = L"";
		// 
		// dataGridViewPT
		// 
		this->dataGridViewPT->AllowUserToAddRows = false;
		this->dataGridViewPT->AllowUserToDeleteRows = false;
		this->dataGridViewPT->AllowUserToResizeRows = false;
		this->dataGridViewPT->ColumnHeadersHeightSizeMode = System::Windows::Forms::DataGridViewColumnHeadersHeightSizeMode::AutoSize;
		this->dataGridViewPT->Columns->AddRange(gcnew cli::array< System::Windows::Forms::DataGridViewColumn^  >(2) {
			this->Index4,
				this->Value4
		});
		this->dataGridViewPT->Location = System::Drawing::Point(645, 155);
		this->dataGridViewPT->MultiSelect = false;
		this->dataGridViewPT->Name = L"dataGridViewPT";
		this->dataGridViewPT->ReadOnly = true;
		this->dataGridViewPT->RowHeadersVisible = false;
		this->dataGridViewPT->SelectionMode = System::Windows::Forms::DataGridViewSelectionMode::FullRowSelect;
		this->dataGridViewPT->Size = System::Drawing::Size(150, 302);
		this->dataGridViewPT->TabIndex = 29;
		this->dataGridViewPT->CellClick += gcnew System::Windows::Forms::DataGridViewCellEventHandler(this, &MainForm::DataGridViewPT_CellClick);
		// 
		// Index4
		// 
		this->Index4->AutoSizeMode = System::Windows::Forms::DataGridViewAutoSizeColumnMode::None;
		this->Index4->HeaderText = L"Index";
		this->Index4->Name = L"Index4";
		this->Index4->ReadOnly = true;
		this->Index4->Resizable = System::Windows::Forms::DataGridViewTriState::False;
		this->Index4->SortMode = System::Windows::Forms::DataGridViewColumnSortMode::NotSortable;
		this->Index4->Width = 39;
		// 
		// Value4
		// 
		this->Value4->AutoSizeMode = System::Windows::Forms::DataGridViewAutoSizeColumnMode::Fill;
		this->Value4->HeaderText = L"Value";
		this->Value4->Name = L"Value4";
		this->Value4->ReadOnly = true;
		this->Value4->Resizable = System::Windows::Forms::DataGridViewTriState::False;
		this->Value4->SortMode = System::Windows::Forms::DataGridViewColumnSortMode::NotSortable;
		// 
		// dataGridViewPDE
		// 
		this->dataGridViewPDE->AllowUserToAddRows = false;
		this->dataGridViewPDE->AllowUserToDeleteRows = false;
		this->dataGridViewPDE->AllowUserToResizeRows = false;
		this->dataGridViewPDE->ColumnHeadersHeightSizeMode = System::Windows::Forms::DataGridViewColumnHeadersHeightSizeMode::AutoSize;
		this->dataGridViewPDE->Columns->AddRange(gcnew cli::array< System::Windows::Forms::DataGridViewColumn^  >(2) {
			this->Index3,
				this->Value3
		});
		this->dataGridViewPDE->Location = System::Drawing::Point(446, 155);
		this->dataGridViewPDE->MultiSelect = false;
		this->dataGridViewPDE->Name = L"dataGridViewPDE";
		this->dataGridViewPDE->ReadOnly = true;
		this->dataGridViewPDE->RowHeadersVisible = false;
		this->dataGridViewPDE->SelectionMode = System::Windows::Forms::DataGridViewSelectionMode::FullRowSelect;
		this->dataGridViewPDE->Size = System::Drawing::Size(150, 302);
		this->dataGridViewPDE->TabIndex = 28;
		this->dataGridViewPDE->CellClick += gcnew System::Windows::Forms::DataGridViewCellEventHandler(this, &MainForm::DataGridViewPDE_CellClick);
		// 
		// Index3
		// 
		this->Index3->HeaderText = L"Index";
		this->Index3->Name = L"Index3";
		this->Index3->ReadOnly = true;
		this->Index3->Resizable = System::Windows::Forms::DataGridViewTriState::False;
		this->Index3->SortMode = System::Windows::Forms::DataGridViewColumnSortMode::NotSortable;
		this->Index3->Width = 39;
		// 
		// Value3
		// 
		this->Value3->AutoSizeMode = System::Windows::Forms::DataGridViewAutoSizeColumnMode::Fill;
		this->Value3->HeaderText = L"Value";
		this->Value3->Name = L"Value3";
		this->Value3->ReadOnly = true;
		this->Value3->Resizable = System::Windows::Forms::DataGridViewTriState::False;
		this->Value3->SortMode = System::Windows::Forms::DataGridViewColumnSortMode::NotSortable;
		// 
		// dataGridViewPDP
		// 
		this->dataGridViewPDP->AllowUserToAddRows = false;
		this->dataGridViewPDP->AllowUserToDeleteRows = false;
		this->dataGridViewPDP->AllowUserToResizeRows = false;
		this->dataGridViewPDP->ColumnHeadersHeightSizeMode = System::Windows::Forms::DataGridViewColumnHeadersHeightSizeMode::AutoSize;
		this->dataGridViewPDP->Columns->AddRange(gcnew cli::array< System::Windows::Forms::DataGridViewColumn^  >(2) {
			this->Index2,
				this->Value2
		});
		this->dataGridViewPDP->Location = System::Drawing::Point(246, 155);
		this->dataGridViewPDP->MultiSelect = false;
		this->dataGridViewPDP->Name = L"dataGridViewPDP";
		this->dataGridViewPDP->ReadOnly = true;
		this->dataGridViewPDP->RowHeadersVisible = false;
		this->dataGridViewPDP->SelectionMode = System::Windows::Forms::DataGridViewSelectionMode::FullRowSelect;
		this->dataGridViewPDP->Size = System::Drawing::Size(150, 302);
		this->dataGridViewPDP->TabIndex = 27;
		this->dataGridViewPDP->CellClick += gcnew System::Windows::Forms::DataGridViewCellEventHandler(this, &MainForm::DataGridViewPDP_CellClick);
		// 
		// Index2
		// 
		this->Index2->AutoSizeMode = System::Windows::Forms::DataGridViewAutoSizeColumnMode::None;
		this->Index2->HeaderText = L"Index";
		this->Index2->Name = L"Index2";
		this->Index2->ReadOnly = true;
		this->Index2->Resizable = System::Windows::Forms::DataGridViewTriState::False;
		this->Index2->SortMode = System::Windows::Forms::DataGridViewColumnSortMode::NotSortable;
		this->Index2->Width = 39;
		// 
		// Value2
		// 
		this->Value2->AutoSizeMode = System::Windows::Forms::DataGridViewAutoSizeColumnMode::Fill;
		this->Value2->HeaderText = L"Value";
		this->Value2->Name = L"Value2";
		this->Value2->ReadOnly = true;
		this->Value2->Resizable = System::Windows::Forms::DataGridViewTriState::False;
		this->Value2->SortMode = System::Windows::Forms::DataGridViewColumnSortMode::NotSortable;
		// 
		// dataGridViewPML4
		// 
		this->dataGridViewPML4->AllowUserToAddRows = false;
		this->dataGridViewPML4->AllowUserToDeleteRows = false;
		this->dataGridViewPML4->AllowUserToResizeColumns = false;
		this->dataGridViewPML4->AllowUserToResizeRows = false;
		this->dataGridViewPML4->ColumnHeadersHeightSizeMode = System::Windows::Forms::DataGridViewColumnHeadersHeightSizeMode::AutoSize;
		this->dataGridViewPML4->Columns->AddRange(gcnew cli::array< System::Windows::Forms::DataGridViewColumn^  >(2) {
			this->Index,
				this->Value
		});
		this->dataGridViewPML4->Location = System::Drawing::Point(47, 155);
		this->dataGridViewPML4->MultiSelect = false;
		this->dataGridViewPML4->Name = L"dataGridViewPML4";
		this->dataGridViewPML4->ReadOnly = true;
		this->dataGridViewPML4->RowHeadersVisible = false;
		this->dataGridViewPML4->SelectionMode = System::Windows::Forms::DataGridViewSelectionMode::FullRowSelect;
		this->dataGridViewPML4->Size = System::Drawing::Size(150, 302);
		this->dataGridViewPML4->TabIndex = 26;
		this->dataGridViewPML4->CellClick += gcnew System::Windows::Forms::DataGridViewCellEventHandler(this, &MainForm::DataGridViewPML4_CellClick);
		// 
		// Index
		// 
		this->Index->AutoSizeMode = System::Windows::Forms::DataGridViewAutoSizeColumnMode::None;
		this->Index->HeaderText = L"Index";
		this->Index->Name = L"Index";
		this->Index->ReadOnly = true;
		this->Index->Resizable = System::Windows::Forms::DataGridViewTriState::False;
		this->Index->SortMode = System::Windows::Forms::DataGridViewColumnSortMode::NotSortable;
		this->Index->Width = 39;
		// 
		// Value
		// 
		this->Value->AutoSizeMode = System::Windows::Forms::DataGridViewAutoSizeColumnMode::Fill;
		this->Value->HeaderText = L"Value";
		this->Value->Name = L"Value";
		this->Value->ReadOnly = true;
		this->Value->Resizable = System::Windows::Forms::DataGridViewTriState::False;
		this->Value->SortMode = System::Windows::Forms::DataGridViewColumnSortMode::NotSortable;
		// 
		// Refresh_button
		// 
		this->Refresh_button->Location = System::Drawing::Point(226, 27);
		this->Refresh_button->Name = L"Refresh_button";
		this->Refresh_button->Size = System::Drawing::Size(53, 23);
		this->Refresh_button->TabIndex = 3;
		this->Refresh_button->Text = L"Refresh";
		this->Refresh_button->UseVisualStyleBackColor = true;
		this->Refresh_button->Click += gcnew System::EventHandler(this, &MainForm::RefreshButton_Click);
		// 
		// statusStrip
		// 
		this->statusStrip->ImageScalingSize = System::Drawing::Size(32, 32);
		this->statusStrip->Items->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(1) { this->stripStatusLabel });
		this->statusStrip->Location = System::Drawing::Point(0, 1003);
		this->statusStrip->Name = L"statusStrip";
		this->statusStrip->Size = System::Drawing::Size(1362, 22);
		this->statusStrip->TabIndex = 8;
		this->statusStrip->Text = L"statusStrip1";
		// 
		// stripStatusLabel
		// 
		this->stripStatusLabel->Name = L"stripStatusLabel";
		this->stripStatusLabel->Size = System::Drawing::Size(113, 17);
		this->stripStatusLabel->Text = L"___Current_status___";
		// 
		// dgvProcesses
		// 
		this->dgvProcesses->AllowUserToAddRows = false;
		this->dgvProcesses->AllowUserToDeleteRows = false;
		this->dgvProcesses->AllowUserToOrderColumns = true;
		this->dgvProcesses->AllowUserToResizeRows = false;
		this->dgvProcesses->Anchor = static_cast<System::Windows::Forms::AnchorStyles>(((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Bottom)
			| System::Windows::Forms::AnchorStyles::Left));
		this->dgvProcesses->ColumnHeadersHeightSizeMode = System::Windows::Forms::DataGridViewColumnHeadersHeightSizeMode::AutoSize;
		this->dgvProcesses->Columns->AddRange(gcnew cli::array< System::Windows::Forms::DataGridViewColumn^  >(2) { this->PID, this->ProcessName });
		this->dgvProcesses->Location = System::Drawing::Point(0, 56);
		this->dgvProcesses->MultiSelect = false;
		this->dgvProcesses->Name = L"dgvProcesses";
		this->dgvProcesses->ReadOnly = true;
		this->dgvProcesses->RowHeadersVisible = false;
		this->dgvProcesses->SelectionMode = System::Windows::Forms::DataGridViewSelectionMode::FullRowSelect;
		this->dgvProcesses->Size = System::Drawing::Size(279, 944);
		this->dgvProcesses->TabIndex = 11;
		this->dgvProcesses->TabStop = false;
		this->dgvProcesses->CellClick += gcnew System::Windows::Forms::DataGridViewCellEventHandler(this, &MainForm::DgvProcesses_CellContentClick);
		// 
		// PID
		// 
		this->PID->AutoSizeMode = System::Windows::Forms::DataGridViewAutoSizeColumnMode::None;
		dataGridViewCellStyle1->Alignment = System::Windows::Forms::DataGridViewContentAlignment::MiddleLeft;
		this->PID->DefaultCellStyle = dataGridViewCellStyle1;
		this->PID->HeaderText = L"PID";
		this->PID->Name = L"PID";
		this->PID->ReadOnly = true;
		this->PID->Resizable = System::Windows::Forms::DataGridViewTriState::True;
		this->PID->Width = 50;
		// 
		// ProcessName
		// 
		this->ProcessName->AutoSizeMode = System::Windows::Forms::DataGridViewAutoSizeColumnMode::Fill;
		dataGridViewCellStyle2->Alignment = System::Windows::Forms::DataGridViewContentAlignment::MiddleLeft;
		this->ProcessName->DefaultCellStyle = dataGridViewCellStyle2;
		this->ProcessName->HeaderText = L"ProcessName";
		this->ProcessName->Name = L"ProcessName";
		this->ProcessName->ReadOnly = true;
		this->ProcessName->Resizable = System::Windows::Forms::DataGridViewTriState::True;
		// 
		// dgvMemory
		// 
		this->dgvMemory->AllowUserToAddRows = false;
		this->dgvMemory->AllowUserToDeleteRows = false;
		this->dgvMemory->AllowUserToOrderColumns = true;
		this->dgvMemory->AllowUserToResizeRows = false;
		this->dgvMemory->Anchor = static_cast<System::Windows::Forms::AnchorStyles>(((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Bottom)
			| System::Windows::Forms::AnchorStyles::Left));
		this->dgvMemory->ColumnHeadersHeightSizeMode = System::Windows::Forms::DataGridViewColumnHeadersHeightSizeMode::AutoSize;
		this->dgvMemory->Columns->AddRange(gcnew cli::array< System::Windows::Forms::DataGridViewColumn^  >(2) {
			this->BaseAddress,
				this->Protection
		});
		this->dgvMemory->Location = System::Drawing::Point(285, 27);
		this->dgvMemory->MultiSelect = false;
		this->dgvMemory->Name = L"dgvMemory";
		this->dgvMemory->ReadOnly = true;
		this->dgvMemory->RowHeadersVisible = false;
		this->dgvMemory->SelectionMode = System::Windows::Forms::DataGridViewSelectionMode::FullRowSelect;
		this->dgvMemory->Size = System::Drawing::Size(224, 973);
		this->dgvMemory->TabIndex = 12;
		this->dgvMemory->CellClick += gcnew System::Windows::Forms::DataGridViewCellEventHandler(this, &MainForm::DgvMemory_CellContentClick);
		// 
		// BaseAddress
		// 
		this->BaseAddress->AutoSizeMode = System::Windows::Forms::DataGridViewAutoSizeColumnMode::None;
		this->BaseAddress->HeaderText = L"BaseAddress";
		this->BaseAddress->Name = L"BaseAddress";
		this->BaseAddress->ReadOnly = true;
		this->BaseAddress->Resizable = System::Windows::Forms::DataGridViewTriState::False;
		this->BaseAddress->SortMode = System::Windows::Forms::DataGridViewColumnSortMode::NotSortable;
		// 
		// Protection
		// 
		this->Protection->AutoSizeMode = System::Windows::Forms::DataGridViewAutoSizeColumnMode::Fill;
		this->Protection->HeaderText = L"Protection";
		this->Protection->Name = L"Protection";
		this->Protection->ReadOnly = true;
		this->Protection->Resizable = System::Windows::Forms::DataGridViewTriState::False;
		this->Protection->SortMode = System::Windows::Forms::DataGridViewColumnSortMode::NotSortable;
		// 
		// menuStrip1
		// 
		this->menuStrip1->ImageScalingSize = System::Drawing::Size(32, 32);
		this->menuStrip1->Items->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(2) {
			this->menu1ToolStripMenuItem,
				this->aboutToolStripMenuItem
		});
		this->menuStrip1->Location = System::Drawing::Point(0, 0);
		this->menuStrip1->Name = L"menuStrip1";
		this->menuStrip1->Size = System::Drawing::Size(1362, 24);
		this->menuStrip1->TabIndex = 16;
		this->menuStrip1->Text = L"menuStrip1";
		// 
		// menu1ToolStripMenuItem
		// 
		this->menu1ToolStripMenuItem->DropDownItems->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(2) {
			this->RefreshStripMenuItem,
				this->exitToolStripMenuItem
		});
		this->menu1ToolStripMenuItem->Name = L"menu1ToolStripMenuItem";
		this->menu1ToolStripMenuItem->Size = System::Drawing::Size(50, 20);
		this->menu1ToolStripMenuItem->Text = L"Menu";
		// 
		// RefreshStripMenuItem
		// 
		this->RefreshStripMenuItem->Name = L"RefreshStripMenuItem";
		this->RefreshStripMenuItem->Size = System::Drawing::Size(180, 22);
		this->RefreshStripMenuItem->Text = L"Refresh";
		this->RefreshStripMenuItem->Click += gcnew System::EventHandler(this, &MainForm::RefreshToolStripMenuItem_Click);
		// 
		// exitToolStripMenuItem
		// 
		this->exitToolStripMenuItem->Name = L"exitToolStripMenuItem";
		this->exitToolStripMenuItem->Size = System::Drawing::Size(180, 22);
		this->exitToolStripMenuItem->Text = L"Exit";
		this->exitToolStripMenuItem->Click += gcnew System::EventHandler(this, &MainForm::exitToolStripMenuItem_Click);
		// 
		// aboutToolStripMenuItem
		// 
		this->aboutToolStripMenuItem->DropDownItems->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(1) { this->aboutToolStripMenuItem1 });
		this->aboutToolStripMenuItem->Name = L"aboutToolStripMenuItem";
		this->aboutToolStripMenuItem->Size = System::Drawing::Size(44, 20);
		this->aboutToolStripMenuItem->Text = L"Help";
		// 
		// aboutToolStripMenuItem1
		// 
		this->aboutToolStripMenuItem1->Name = L"aboutToolStripMenuItem1";
		this->aboutToolStripMenuItem1->Size = System::Drawing::Size(107, 22);
		this->aboutToolStripMenuItem1->Text = L"About";
		this->aboutToolStripMenuItem1->Click += gcnew System::EventHandler(this, &MainForm::aboutToolStripMenuItem1_Click);
		// 
		// label1
		// 
		this->label1->AutoSize = true;
		this->label1->Location = System::Drawing::Point(12, 32);
		this->label1->Name = L"label1";
		this->label1->Size = System::Drawing::Size(59, 13);
		this->label1->TabIndex = 13;
		this->label1->Text = L"Processes:";
		// 
		// AddressTab
		// 
		this->AddressTab->BackColor = System::Drawing::SystemColors::Control;
		this->AddressTab->Controls->Add(this->addressListBox);
		this->AddressTab->Location = System::Drawing::Point(4, 22);
		this->AddressTab->Name = L"AddressTab";
		this->AddressTab->Padding = System::Windows::Forms::Padding(3);
		this->AddressTab->Size = System::Drawing::Size(832, 265);
		this->AddressTab->TabIndex = 5;
		this->AddressTab->Text = L"Address";
		// 
		// addressListBox
		// 
		this->addressListBox->FormattingEnabled = true;
		this->addressListBox->Location = System::Drawing::Point(44, 6);
		this->addressListBox->Name = L"addressListBox";
		this->addressListBox->Size = System::Drawing::Size(590, 199);
		this->addressListBox->TabIndex = 33;
		// 
		// RegistersTab
		// 
		this->RegistersTab->BackColor = System::Drawing::SystemColors::Control;
		this->RegistersTab->Controls->Add(this->dataGridCR3);
		this->RegistersTab->Controls->Add(this->labelCR2);
		this->RegistersTab->Controls->Add(this->dataGridCR0);
		this->RegistersTab->Location = System::Drawing::Point(4, 22);
		this->RegistersTab->Name = L"RegistersTab";
		this->RegistersTab->Padding = System::Windows::Forms::Padding(3);
		this->RegistersTab->Size = System::Drawing::Size(832, 265);
		this->RegistersTab->TabIndex = 2;
		this->RegistersTab->Text = L"Registers";
		// 
		// dataGridCR3
		// 
		this->dataGridCR3->AllowUserToAddRows = false;
		this->dataGridCR3->AllowUserToDeleteRows = false;
		this->dataGridCR3->AllowUserToOrderColumns = true;
		this->dataGridCR3->AllowUserToResizeRows = false;
		this->dataGridCR3->AutoSizeRowsMode = System::Windows::Forms::DataGridViewAutoSizeRowsMode::AllCells;
		this->dataGridCR3->ColumnHeadersHeightSizeMode = System::Windows::Forms::DataGridViewColumnHeadersHeightSizeMode::AutoSize;
		this->dataGridCR3->Columns->AddRange(gcnew cli::array< System::Windows::Forms::DataGridViewColumn^  >(7) {
			this->dataGridViewTextBoxColumn1,
				this->Column17, this->dataGridViewTextBoxColumn2, this->dataGridViewTextBoxColumn3, this->dataGridViewTextBoxColumn4, this->dataGridViewTextBoxColumn5,
				this->dataGridViewTextBoxColumn6
		});
		this->dataGridCR3->Enabled = false;
		this->dataGridCR3->Location = System::Drawing::Point(44, 80);
		this->dataGridCR3->MultiSelect = false;
		this->dataGridCR3->Name = L"dataGridCR3";
		this->dataGridCR3->ReadOnly = true;
		this->dataGridCR3->RowHeadersVisible = false;
		this->dataGridCR3->Size = System::Drawing::Size(444, 73);
		this->dataGridCR3->TabIndex = 32;
		// 
		// dataGridViewTextBoxColumn1
		// 
		this->dataGridViewTextBoxColumn1->AutoSizeMode = System::Windows::Forms::DataGridViewAutoSizeColumnMode::None;
		dataGridViewCellStyle3->Alignment = System::Windows::Forms::DataGridViewContentAlignment::MiddleLeft;
		this->dataGridViewTextBoxColumn1->DefaultCellStyle = dataGridViewCellStyle3;
		this->dataGridViewTextBoxColumn1->HeaderText = L"Table";
		this->dataGridViewTextBoxColumn1->Name = L"dataGridViewTextBoxColumn1";
		this->dataGridViewTextBoxColumn1->ReadOnly = true;
		this->dataGridViewTextBoxColumn1->Resizable = System::Windows::Forms::DataGridViewTriState::False;
		this->dataGridViewTextBoxColumn1->SortMode = System::Windows::Forms::DataGridViewColumnSortMode::NotSortable;
		this->dataGridViewTextBoxColumn1->Width = 40;
		// 
		// Column17
		// 
		dataGridViewCellStyle4->Alignment = System::Windows::Forms::DataGridViewContentAlignment::MiddleCenter;
		this->Column17->DefaultCellStyle = dataGridViewCellStyle4;
		this->Column17->HeaderText = L"63...49";
		this->Column17->Name = L"Column17";
		this->Column17->ReadOnly = true;
		this->Column17->Width = 80;
		// 
		// dataGridViewTextBoxColumn2
		// 
		dataGridViewCellStyle5->Alignment = System::Windows::Forms::DataGridViewContentAlignment::MiddleCenter;
		this->dataGridViewTextBoxColumn2->DefaultCellStyle = dataGridViewCellStyle5;
		this->dataGridViewTextBoxColumn2->HeaderText = L"48...12";
		this->dataGridViewTextBoxColumn2->Name = L"dataGridViewTextBoxColumn2";
		this->dataGridViewTextBoxColumn2->ReadOnly = true;
		this->dataGridViewTextBoxColumn2->Width = 80;
		// 
		// dataGridViewTextBoxColumn3
		// 
		this->dataGridViewTextBoxColumn3->AutoSizeMode = System::Windows::Forms::DataGridViewAutoSizeColumnMode::None;
		dataGridViewCellStyle6->Alignment = System::Windows::Forms::DataGridViewContentAlignment::MiddleCenter;
		this->dataGridViewTextBoxColumn3->DefaultCellStyle = dataGridViewCellStyle6;
		this->dataGridViewTextBoxColumn3->HeaderText = L"11...5";
		this->dataGridViewTextBoxColumn3->Name = L"dataGridViewTextBoxColumn3";
		this->dataGridViewTextBoxColumn3->ReadOnly = true;
		this->dataGridViewTextBoxColumn3->Resizable = System::Windows::Forms::DataGridViewTriState::False;
		this->dataGridViewTextBoxColumn3->SortMode = System::Windows::Forms::DataGridViewColumnSortMode::NotSortable;
		this->dataGridViewTextBoxColumn3->Width = 80;
		// 
		// dataGridViewTextBoxColumn4
		// 
		this->dataGridViewTextBoxColumn4->AutoSizeMode = System::Windows::Forms::DataGridViewAutoSizeColumnMode::None;
		dataGridViewCellStyle7->Alignment = System::Windows::Forms::DataGridViewContentAlignment::MiddleCenter;
		dataGridViewCellStyle7->WrapMode = System::Windows::Forms::DataGridViewTriState::True;
		this->dataGridViewTextBoxColumn4->DefaultCellStyle = dataGridViewCellStyle7;
		this->dataGridViewTextBoxColumn4->HeaderText = L"4";
		this->dataGridViewTextBoxColumn4->Name = L"dataGridViewTextBoxColumn4";
		this->dataGridViewTextBoxColumn4->ReadOnly = true;
		this->dataGridViewTextBoxColumn4->Resizable = System::Windows::Forms::DataGridViewTriState::False;
		this->dataGridViewTextBoxColumn4->SortMode = System::Windows::Forms::DataGridViewColumnSortMode::NotSortable;
		this->dataGridViewTextBoxColumn4->Width = 40;
		// 
		// dataGridViewTextBoxColumn5
		// 
		this->dataGridViewTextBoxColumn5->AutoSizeMode = System::Windows::Forms::DataGridViewAutoSizeColumnMode::None;
		dataGridViewCellStyle8->Alignment = System::Windows::Forms::DataGridViewContentAlignment::MiddleCenter;
		dataGridViewCellStyle8->WrapMode = System::Windows::Forms::DataGridViewTriState::True;
		this->dataGridViewTextBoxColumn5->DefaultCellStyle = dataGridViewCellStyle8;
		this->dataGridViewTextBoxColumn5->HeaderText = L"3";
		this->dataGridViewTextBoxColumn5->Name = L"dataGridViewTextBoxColumn5";
		this->dataGridViewTextBoxColumn5->ReadOnly = true;
		this->dataGridViewTextBoxColumn5->Resizable = System::Windows::Forms::DataGridViewTriState::False;
		this->dataGridViewTextBoxColumn5->SortMode = System::Windows::Forms::DataGridViewColumnSortMode::NotSortable;
		this->dataGridViewTextBoxColumn5->Width = 40;
		// 
		// dataGridViewTextBoxColumn6
		// 
		this->dataGridViewTextBoxColumn6->AutoSizeMode = System::Windows::Forms::DataGridViewAutoSizeColumnMode::Fill;
		dataGridViewCellStyle9->Alignment = System::Windows::Forms::DataGridViewContentAlignment::MiddleCenter;
		dataGridViewCellStyle9->WrapMode = System::Windows::Forms::DataGridViewTriState::True;
		this->dataGridViewTextBoxColumn6->DefaultCellStyle = dataGridViewCellStyle9;
		this->dataGridViewTextBoxColumn6->HeaderText = L"2...0";
		this->dataGridViewTextBoxColumn6->Name = L"dataGridViewTextBoxColumn6";
		this->dataGridViewTextBoxColumn6->ReadOnly = true;
		this->dataGridViewTextBoxColumn6->Resizable = System::Windows::Forms::DataGridViewTriState::False;
		this->dataGridViewTextBoxColumn6->SortMode = System::Windows::Forms::DataGridViewColumnSortMode::NotSortable;
		// 
		// labelCR2
		// 
		this->labelCR2->AutoSize = true;
		this->labelCR2->Location = System::Drawing::Point(41, 156);
		this->labelCR2->Name = L"labelCR2";
		this->labelCR2->Size = System::Drawing::Size(34, 13);
		this->labelCR2->TabIndex = 31;
		this->labelCR2->Text = L"CR2: ";
		// 
		// dataGridCR0
		// 
		this->dataGridCR0->AllowUserToAddRows = false;
		this->dataGridCR0->AllowUserToDeleteRows = false;
		this->dataGridCR0->AllowUserToOrderColumns = true;
		this->dataGridCR0->AllowUserToResizeRows = false;
		this->dataGridCR0->AutoSizeRowsMode = System::Windows::Forms::DataGridViewAutoSizeRowsMode::AllCells;
		this->dataGridCR0->ColumnHeadersHeightSizeMode = System::Windows::Forms::DataGridViewColumnHeadersHeightSizeMode::AutoSize;
		this->dataGridCR0->Columns->AddRange(gcnew cli::array< System::Windows::Forms::DataGridViewColumn^  >(16) {
			this->dataGridViewTextBoxColumn7,
				this->Column16, this->Column15, this->dataGridViewTextBoxColumn8, this->dataGridViewTextBoxColumn9, this->dataGridViewTextBoxColumn10,
				this->dataGridViewTextBoxColumn11, this->dataGridViewTextBoxColumn12, this->dataGridViewTextBoxColumn13, this->dataGridViewTextBoxColumn14,
				this->dataGridViewTextBoxColumn15, this->dataGridViewTextBoxColumn16, this->dataGridViewTextBoxColumn17, this->dataGridViewTextBoxColumn18,
				this->dataGridViewTextBoxColumn19, this->dataGridViewTextBoxColumn20
		});
		this->dataGridCR0->Enabled = false;
		this->dataGridCR0->Location = System::Drawing::Point(44, 3);
		this->dataGridCR0->MultiSelect = false;
		this->dataGridCR0->Name = L"dataGridCR0";
		this->dataGridCR0->ReadOnly = true;
		this->dataGridCR0->RowHeadersVisible = false;
		this->dataGridCR0->Size = System::Drawing::Size(586, 73);
		this->dataGridCR0->TabIndex = 30;
		// 
		// dataGridViewTextBoxColumn7
		// 
		this->dataGridViewTextBoxColumn7->AutoSizeMode = System::Windows::Forms::DataGridViewAutoSizeColumnMode::None;
		dataGridViewCellStyle10->Alignment = System::Windows::Forms::DataGridViewContentAlignment::MiddleLeft;
		this->dataGridViewTextBoxColumn7->DefaultCellStyle = dataGridViewCellStyle10;
		this->dataGridViewTextBoxColumn7->HeaderText = L"Table";
		this->dataGridViewTextBoxColumn7->Name = L"dataGridViewTextBoxColumn7";
		this->dataGridViewTextBoxColumn7->ReadOnly = true;
		this->dataGridViewTextBoxColumn7->Resizable = System::Windows::Forms::DataGridViewTriState::False;
		this->dataGridViewTextBoxColumn7->SortMode = System::Windows::Forms::DataGridViewColumnSortMode::NotSortable;
		this->dataGridViewTextBoxColumn7->Width = 40;
		// 
		// Column16
		// 
		dataGridViewCellStyle11->Alignment = System::Windows::Forms::DataGridViewContentAlignment::MiddleCenter;
		this->Column16->DefaultCellStyle = dataGridViewCellStyle11;
		this->Column16->HeaderText = L"63..32";
		this->Column16->Name = L"Column16";
		this->Column16->ReadOnly = true;
		this->Column16->Width = 70;
		// 
		// Column15
		// 
		this->Column15->AutoSizeMode = System::Windows::Forms::DataGridViewAutoSizeColumnMode::None;
		dataGridViewCellStyle12->Alignment = System::Windows::Forms::DataGridViewContentAlignment::MiddleCenter;
		this->Column15->DefaultCellStyle = dataGridViewCellStyle12;
		this->Column15->HeaderText = L"31";
		this->Column15->Name = L"Column15";
		this->Column15->ReadOnly = true;
		this->Column15->Resizable = System::Windows::Forms::DataGridViewTriState::False;
		this->Column15->SortMode = System::Windows::Forms::DataGridViewColumnSortMode::NotSortable;
		this->Column15->Width = 22;
		// 
		// dataGridViewTextBoxColumn8
		// 
		this->dataGridViewTextBoxColumn8->AutoSizeMode = System::Windows::Forms::DataGridViewAutoSizeColumnMode::None;
		dataGridViewCellStyle13->Alignment = System::Windows::Forms::DataGridViewContentAlignment::MiddleCenter;
		dataGridViewCellStyle13->WrapMode = System::Windows::Forms::DataGridViewTriState::True;
		this->dataGridViewTextBoxColumn8->DefaultCellStyle = dataGridViewCellStyle13;
		this->dataGridViewTextBoxColumn8->HeaderText = L"30";
		this->dataGridViewTextBoxColumn8->Name = L"dataGridViewTextBoxColumn8";
		this->dataGridViewTextBoxColumn8->ReadOnly = true;
		this->dataGridViewTextBoxColumn8->Resizable = System::Windows::Forms::DataGridViewTriState::False;
		this->dataGridViewTextBoxColumn8->SortMode = System::Windows::Forms::DataGridViewColumnSortMode::NotSortable;
		this->dataGridViewTextBoxColumn8->Width = 22;
		// 
		// dataGridViewTextBoxColumn9
		// 
		this->dataGridViewTextBoxColumn9->AutoSizeMode = System::Windows::Forms::DataGridViewAutoSizeColumnMode::None;
		dataGridViewCellStyle14->Alignment = System::Windows::Forms::DataGridViewContentAlignment::MiddleCenter;
		dataGridViewCellStyle14->WrapMode = System::Windows::Forms::DataGridViewTriState::True;
		this->dataGridViewTextBoxColumn9->DefaultCellStyle = dataGridViewCellStyle14;
		this->dataGridViewTextBoxColumn9->HeaderText = L"29";
		this->dataGridViewTextBoxColumn9->Name = L"dataGridViewTextBoxColumn9";
		this->dataGridViewTextBoxColumn9->ReadOnly = true;
		this->dataGridViewTextBoxColumn9->Resizable = System::Windows::Forms::DataGridViewTriState::False;
		this->dataGridViewTextBoxColumn9->SortMode = System::Windows::Forms::DataGridViewColumnSortMode::NotSortable;
		this->dataGridViewTextBoxColumn9->Width = 22;
		// 
		// dataGridViewTextBoxColumn10
		// 
		this->dataGridViewTextBoxColumn10->AutoSizeMode = System::Windows::Forms::DataGridViewAutoSizeColumnMode::None;
		dataGridViewCellStyle15->Alignment = System::Windows::Forms::DataGridViewContentAlignment::MiddleCenter;
		dataGridViewCellStyle15->WrapMode = System::Windows::Forms::DataGridViewTriState::True;
		this->dataGridViewTextBoxColumn10->DefaultCellStyle = dataGridViewCellStyle15;
		this->dataGridViewTextBoxColumn10->HeaderText = L"28...19";
		this->dataGridViewTextBoxColumn10->Name = L"dataGridViewTextBoxColumn10";
		this->dataGridViewTextBoxColumn10->ReadOnly = true;
		this->dataGridViewTextBoxColumn10->Resizable = System::Windows::Forms::DataGridViewTriState::False;
		this->dataGridViewTextBoxColumn10->SortMode = System::Windows::Forms::DataGridViewColumnSortMode::NotSortable;
		this->dataGridViewTextBoxColumn10->Width = 70;
		// 
		// dataGridViewTextBoxColumn11
		// 
		this->dataGridViewTextBoxColumn11->AutoSizeMode = System::Windows::Forms::DataGridViewAutoSizeColumnMode::None;
		dataGridViewCellStyle16->Alignment = System::Windows::Forms::DataGridViewContentAlignment::MiddleCenter;
		dataGridViewCellStyle16->WrapMode = System::Windows::Forms::DataGridViewTriState::True;
		this->dataGridViewTextBoxColumn11->DefaultCellStyle = dataGridViewCellStyle16;
		this->dataGridViewTextBoxColumn11->HeaderText = L"18";
		this->dataGridViewTextBoxColumn11->Name = L"dataGridViewTextBoxColumn11";
		this->dataGridViewTextBoxColumn11->ReadOnly = true;
		this->dataGridViewTextBoxColumn11->Resizable = System::Windows::Forms::DataGridViewTriState::False;
		this->dataGridViewTextBoxColumn11->SortMode = System::Windows::Forms::DataGridViewColumnSortMode::NotSortable;
		this->dataGridViewTextBoxColumn11->Width = 70;
		// 
		// dataGridViewTextBoxColumn12
		// 
		this->dataGridViewTextBoxColumn12->AutoSizeMode = System::Windows::Forms::DataGridViewAutoSizeColumnMode::None;
		dataGridViewCellStyle17->Alignment = System::Windows::Forms::DataGridViewContentAlignment::MiddleCenter;
		dataGridViewCellStyle17->WrapMode = System::Windows::Forms::DataGridViewTriState::True;
		this->dataGridViewTextBoxColumn12->DefaultCellStyle = dataGridViewCellStyle17;
		this->dataGridViewTextBoxColumn12->HeaderText = L"17";
		this->dataGridViewTextBoxColumn12->Name = L"dataGridViewTextBoxColumn12";
		this->dataGridViewTextBoxColumn12->ReadOnly = true;
		this->dataGridViewTextBoxColumn12->Resizable = System::Windows::Forms::DataGridViewTriState::False;
		this->dataGridViewTextBoxColumn12->SortMode = System::Windows::Forms::DataGridViewColumnSortMode::NotSortable;
		this->dataGridViewTextBoxColumn12->Width = 46;
		// 
		// dataGridViewTextBoxColumn13
		// 
		this->dataGridViewTextBoxColumn13->AutoSizeMode = System::Windows::Forms::DataGridViewAutoSizeColumnMode::None;
		dataGridViewCellStyle18->Alignment = System::Windows::Forms::DataGridViewContentAlignment::MiddleCenter;
		dataGridViewCellStyle18->WrapMode = System::Windows::Forms::DataGridViewTriState::True;
		this->dataGridViewTextBoxColumn13->DefaultCellStyle = dataGridViewCellStyle18;
		this->dataGridViewTextBoxColumn13->HeaderText = L"16";
		this->dataGridViewTextBoxColumn13->Name = L"dataGridViewTextBoxColumn13";
		this->dataGridViewTextBoxColumn13->ReadOnly = true;
		this->dataGridViewTextBoxColumn13->Resizable = System::Windows::Forms::DataGridViewTriState::False;
		this->dataGridViewTextBoxColumn13->SortMode = System::Windows::Forms::DataGridViewColumnSortMode::NotSortable;
		this->dataGridViewTextBoxColumn13->Width = 22;
		// 
		// dataGridViewTextBoxColumn14
		// 
		this->dataGridViewTextBoxColumn14->AutoSizeMode = System::Windows::Forms::DataGridViewAutoSizeColumnMode::None;
		dataGridViewCellStyle19->Alignment = System::Windows::Forms::DataGridViewContentAlignment::MiddleCenter;
		dataGridViewCellStyle19->WrapMode = System::Windows::Forms::DataGridViewTriState::True;
		this->dataGridViewTextBoxColumn14->DefaultCellStyle = dataGridViewCellStyle19;
		this->dataGridViewTextBoxColumn14->HeaderText = L"15...6";
		this->dataGridViewTextBoxColumn14->Name = L"dataGridViewTextBoxColumn14";
		this->dataGridViewTextBoxColumn14->ReadOnly = true;
		this->dataGridViewTextBoxColumn14->Resizable = System::Windows::Forms::DataGridViewTriState::False;
		this->dataGridViewTextBoxColumn14->SortMode = System::Windows::Forms::DataGridViewColumnSortMode::NotSortable;
		this->dataGridViewTextBoxColumn14->Width = 70;
		// 
		// dataGridViewTextBoxColumn15
		// 
		this->dataGridViewTextBoxColumn15->AutoSizeMode = System::Windows::Forms::DataGridViewAutoSizeColumnMode::None;
		dataGridViewCellStyle20->Alignment = System::Windows::Forms::DataGridViewContentAlignment::MiddleCenter;
		dataGridViewCellStyle20->WrapMode = System::Windows::Forms::DataGridViewTriState::True;
		this->dataGridViewTextBoxColumn15->DefaultCellStyle = dataGridViewCellStyle20;
		this->dataGridViewTextBoxColumn15->HeaderText = L"5";
		this->dataGridViewTextBoxColumn15->Name = L"dataGridViewTextBoxColumn15";
		this->dataGridViewTextBoxColumn15->ReadOnly = true;
		this->dataGridViewTextBoxColumn15->Resizable = System::Windows::Forms::DataGridViewTriState::False;
		this->dataGridViewTextBoxColumn15->SortMode = System::Windows::Forms::DataGridViewColumnSortMode::NotSortable;
		this->dataGridViewTextBoxColumn15->Width = 19;
		// 
		// dataGridViewTextBoxColumn16
		// 
		this->dataGridViewTextBoxColumn16->AutoSizeMode = System::Windows::Forms::DataGridViewAutoSizeColumnMode::None;
		dataGridViewCellStyle21->Alignment = System::Windows::Forms::DataGridViewContentAlignment::MiddleCenter;
		dataGridViewCellStyle21->WrapMode = System::Windows::Forms::DataGridViewTriState::True;
		this->dataGridViewTextBoxColumn16->DefaultCellStyle = dataGridViewCellStyle21;
		this->dataGridViewTextBoxColumn16->HeaderText = L"4";
		this->dataGridViewTextBoxColumn16->Name = L"dataGridViewTextBoxColumn16";
		this->dataGridViewTextBoxColumn16->ReadOnly = true;
		this->dataGridViewTextBoxColumn16->Resizable = System::Windows::Forms::DataGridViewTriState::False;
		this->dataGridViewTextBoxColumn16->SortMode = System::Windows::Forms::DataGridViewColumnSortMode::NotSortable;
		this->dataGridViewTextBoxColumn16->Width = 19;
		// 
		// dataGridViewTextBoxColumn17
		// 
		this->dataGridViewTextBoxColumn17->AutoSizeMode = System::Windows::Forms::DataGridViewAutoSizeColumnMode::None;
		dataGridViewCellStyle22->Alignment = System::Windows::Forms::DataGridViewContentAlignment::MiddleCenter;
		dataGridViewCellStyle22->WrapMode = System::Windows::Forms::DataGridViewTriState::True;
		this->dataGridViewTextBoxColumn17->DefaultCellStyle = dataGridViewCellStyle22;
		this->dataGridViewTextBoxColumn17->HeaderText = L"3";
		this->dataGridViewTextBoxColumn17->Name = L"dataGridViewTextBoxColumn17";
		this->dataGridViewTextBoxColumn17->ReadOnly = true;
		this->dataGridViewTextBoxColumn17->Resizable = System::Windows::Forms::DataGridViewTriState::False;
		this->dataGridViewTextBoxColumn17->SortMode = System::Windows::Forms::DataGridViewColumnSortMode::NotSortable;
		this->dataGridViewTextBoxColumn17->Width = 19;
		// 
		// dataGridViewTextBoxColumn18
		// 
		this->dataGridViewTextBoxColumn18->AutoSizeMode = System::Windows::Forms::DataGridViewAutoSizeColumnMode::None;
		dataGridViewCellStyle23->Alignment = System::Windows::Forms::DataGridViewContentAlignment::MiddleCenter;
		dataGridViewCellStyle23->WrapMode = System::Windows::Forms::DataGridViewTriState::True;
		this->dataGridViewTextBoxColumn18->DefaultCellStyle = dataGridViewCellStyle23;
		this->dataGridViewTextBoxColumn18->HeaderText = L"2";
		this->dataGridViewTextBoxColumn18->Name = L"dataGridViewTextBoxColumn18";
		this->dataGridViewTextBoxColumn18->ReadOnly = true;
		this->dataGridViewTextBoxColumn18->Resizable = System::Windows::Forms::DataGridViewTriState::False;
		this->dataGridViewTextBoxColumn18->SortMode = System::Windows::Forms::DataGridViewColumnSortMode::NotSortable;
		this->dataGridViewTextBoxColumn18->Width = 19;
		// 
		// dataGridViewTextBoxColumn19
		// 
		this->dataGridViewTextBoxColumn19->AutoSizeMode = System::Windows::Forms::DataGridViewAutoSizeColumnMode::None;
		dataGridViewCellStyle24->Alignment = System::Windows::Forms::DataGridViewContentAlignment::MiddleCenter;
		dataGridViewCellStyle24->WrapMode = System::Windows::Forms::DataGridViewTriState::True;
		this->dataGridViewTextBoxColumn19->DefaultCellStyle = dataGridViewCellStyle24;
		this->dataGridViewTextBoxColumn19->HeaderText = L"1";
		this->dataGridViewTextBoxColumn19->Name = L"dataGridViewTextBoxColumn19";
		this->dataGridViewTextBoxColumn19->ReadOnly = true;
		this->dataGridViewTextBoxColumn19->Resizable = System::Windows::Forms::DataGridViewTriState::False;
		this->dataGridViewTextBoxColumn19->SortMode = System::Windows::Forms::DataGridViewColumnSortMode::NotSortable;
		this->dataGridViewTextBoxColumn19->Width = 19;
		// 
		// dataGridViewTextBoxColumn20
		// 
		this->dataGridViewTextBoxColumn20->AutoSizeMode = System::Windows::Forms::DataGridViewAutoSizeColumnMode::Fill;
		dataGridViewCellStyle25->Alignment = System::Windows::Forms::DataGridViewContentAlignment::MiddleCenter;
		dataGridViewCellStyle25->WrapMode = System::Windows::Forms::DataGridViewTriState::True;
		this->dataGridViewTextBoxColumn20->DefaultCellStyle = dataGridViewCellStyle25;
		this->dataGridViewTextBoxColumn20->HeaderText = L"0";
		this->dataGridViewTextBoxColumn20->Name = L"dataGridViewTextBoxColumn20";
		this->dataGridViewTextBoxColumn20->ReadOnly = true;
		this->dataGridViewTextBoxColumn20->Resizable = System::Windows::Forms::DataGridViewTriState::False;
		this->dataGridViewTextBoxColumn20->SortMode = System::Windows::Forms::DataGridViewColumnSortMode::NotSortable;
		// 
		// TablesTab
		// 
		this->TablesTab->BackColor = System::Drawing::SystemColors::Control;
		this->TablesTab->Controls->Add(this->dgvPTProperties);
		this->TablesTab->Controls->Add(this->dgvProperties);
		this->TablesTab->Location = System::Drawing::Point(4, 22);
		this->TablesTab->Name = L"TablesTab";
		this->TablesTab->Padding = System::Windows::Forms::Padding(3);
		this->TablesTab->Size = System::Drawing::Size(832, 265);
		this->TablesTab->TabIndex = 0;
		this->TablesTab->Text = L"Tables";
		// 
		// dgvPTProperties
		// 
		this->dgvPTProperties->AllowUserToAddRows = false;
		this->dgvPTProperties->AllowUserToDeleteRows = false;
		this->dgvPTProperties->AllowUserToResizeColumns = false;
		this->dgvPTProperties->AllowUserToResizeRows = false;
		this->dgvPTProperties->AutoSizeRowsMode = System::Windows::Forms::DataGridViewAutoSizeRowsMode::AllCells;
		this->dgvPTProperties->ColumnHeadersHeightSizeMode = System::Windows::Forms::DataGridViewColumnHeadersHeightSizeMode::AutoSize;
		this->dgvPTProperties->Columns->AddRange(gcnew cli::array< System::Windows::Forms::DataGridViewColumn^  >(16) {
			this->dataGridViewTextBoxColumn21,
				this->dataGridViewTextBoxColumn22, this->dataGridViewTextBoxColumn23, this->Column19, this->dataGridViewTextBoxColumn24, this->dataGridViewTextBoxColumn25,
				this->dataGridViewTextBoxColumn26, this->Column18, this->dataGridViewTextBoxColumn27, this->dataGridViewTextBoxColumn28, this->dataGridViewTextBoxColumn29,
				this->dataGridViewTextBoxColumn30, this->dataGridViewTextBoxColumn31, this->dataGridViewTextBoxColumn32, this->dataGridViewTextBoxColumn33,
				this->dataGridViewTextBoxColumn34
		});
		this->dgvPTProperties->Enabled = false;
		this->dgvPTProperties->Location = System::Drawing::Point(44, 159);
		this->dgvPTProperties->MultiSelect = false;
		this->dgvPTProperties->Name = L"dgvPTProperties";
		this->dgvPTProperties->ReadOnly = true;
		this->dgvPTProperties->RowHeadersVisible = false;
		this->dgvPTProperties->RowHeadersWidthSizeMode = System::Windows::Forms::DataGridViewRowHeadersWidthSizeMode::DisableResizing;
		this->dgvPTProperties->SelectionMode = System::Windows::Forms::DataGridViewSelectionMode::CellSelect;
		this->dgvPTProperties->Size = System::Drawing::Size(522, 90);
		this->dgvPTProperties->TabIndex = 31;
		// 
		// dataGridViewTextBoxColumn21
		// 
		this->dataGridViewTextBoxColumn21->AutoSizeMode = System::Windows::Forms::DataGridViewAutoSizeColumnMode::None;
		dataGridViewCellStyle26->Alignment = System::Windows::Forms::DataGridViewContentAlignment::MiddleLeft;
		this->dataGridViewTextBoxColumn21->DefaultCellStyle = dataGridViewCellStyle26;
		this->dataGridViewTextBoxColumn21->HeaderText = L"Table";
		this->dataGridViewTextBoxColumn21->Name = L"dataGridViewTextBoxColumn21";
		this->dataGridViewTextBoxColumn21->ReadOnly = true;
		this->dataGridViewTextBoxColumn21->Resizable = System::Windows::Forms::DataGridViewTriState::False;
		this->dataGridViewTextBoxColumn21->SortMode = System::Windows::Forms::DataGridViewColumnSortMode::NotSortable;
		this->dataGridViewTextBoxColumn21->Width = 40;
		// 
		// dataGridViewTextBoxColumn22
		// 
		this->dataGridViewTextBoxColumn22->AutoSizeMode = System::Windows::Forms::DataGridViewAutoSizeColumnMode::None;
		dataGridViewCellStyle27->Alignment = System::Windows::Forms::DataGridViewContentAlignment::MiddleCenter;
		dataGridViewCellStyle27->WrapMode = System::Windows::Forms::DataGridViewTriState::True;
		this->dataGridViewTextBoxColumn22->DefaultCellStyle = dataGridViewCellStyle27;
		this->dataGridViewTextBoxColumn22->HeaderText = L"63";
		this->dataGridViewTextBoxColumn22->Name = L"dataGridViewTextBoxColumn22";
		this->dataGridViewTextBoxColumn22->ReadOnly = true;
		this->dataGridViewTextBoxColumn22->Resizable = System::Windows::Forms::DataGridViewTriState::False;
		this->dataGridViewTextBoxColumn22->SortMode = System::Windows::Forms::DataGridViewColumnSortMode::NotSortable;
		this->dataGridViewTextBoxColumn22->Width = 25;
		// 
		// dataGridViewTextBoxColumn23
		// 
		this->dataGridViewTextBoxColumn23->AutoSizeMode = System::Windows::Forms::DataGridViewAutoSizeColumnMode::None;
		dataGridViewCellStyle28->Alignment = System::Windows::Forms::DataGridViewContentAlignment::MiddleCenter;
		dataGridViewCellStyle28->WrapMode = System::Windows::Forms::DataGridViewTriState::True;
		this->dataGridViewTextBoxColumn23->DefaultCellStyle = dataGridViewCellStyle28;
		this->dataGridViewTextBoxColumn23->HeaderText = L"62 ... 59";
		this->dataGridViewTextBoxColumn23->Name = L"dataGridViewTextBoxColumn23";
		this->dataGridViewTextBoxColumn23->ReadOnly = true;
		this->dataGridViewTextBoxColumn23->Resizable = System::Windows::Forms::DataGridViewTriState::False;
		this->dataGridViewTextBoxColumn23->SortMode = System::Windows::Forms::DataGridViewColumnSortMode::NotSortable;
		this->dataGridViewTextBoxColumn23->Width = 52;
		// 
		// Column19
		// 
		this->Column19->AutoSizeMode = System::Windows::Forms::DataGridViewAutoSizeColumnMode::None;
		dataGridViewCellStyle29->Alignment = System::Windows::Forms::DataGridViewContentAlignment::MiddleCenter;
		this->Column19->DefaultCellStyle = dataGridViewCellStyle29;
		this->Column19->HeaderText = L"58 ... 52";
		this->Column19->Name = L"Column19";
		this->Column19->ReadOnly = true;
		this->Column19->Resizable = System::Windows::Forms::DataGridViewTriState::False;
		this->Column19->SortMode = System::Windows::Forms::DataGridViewColumnSortMode::NotSortable;
		this->Column19->Width = 52;
		// 
		// dataGridViewTextBoxColumn24
		// 
		this->dataGridViewTextBoxColumn24->AutoSizeMode = System::Windows::Forms::DataGridViewAutoSizeColumnMode::None;
		dataGridViewCellStyle30->Alignment = System::Windows::Forms::DataGridViewContentAlignment::MiddleCenter;
		dataGridViewCellStyle30->WrapMode = System::Windows::Forms::DataGridViewTriState::True;
		this->dataGridViewTextBoxColumn24->DefaultCellStyle = dataGridViewCellStyle30;
		this->dataGridViewTextBoxColumn24->HeaderText = L"51 ... M";
		this->dataGridViewTextBoxColumn24->Name = L"dataGridViewTextBoxColumn24";
		this->dataGridViewTextBoxColumn24->ReadOnly = true;
		this->dataGridViewTextBoxColumn24->Resizable = System::Windows::Forms::DataGridViewTriState::False;
		this->dataGridViewTextBoxColumn24->SortMode = System::Windows::Forms::DataGridViewColumnSortMode::NotSortable;
		this->dataGridViewTextBoxColumn24->Width = 55;
		// 
		// dataGridViewTextBoxColumn25
		// 
		this->dataGridViewTextBoxColumn25->AutoSizeMode = System::Windows::Forms::DataGridViewAutoSizeColumnMode::None;
		dataGridViewCellStyle31->Alignment = System::Windows::Forms::DataGridViewContentAlignment::MiddleCenter;
		dataGridViewCellStyle31->WrapMode = System::Windows::Forms::DataGridViewTriState::True;
		this->dataGridViewTextBoxColumn25->DefaultCellStyle = dataGridViewCellStyle31;
		this->dataGridViewTextBoxColumn25->HeaderText = L"M-1 ... 12";
		this->dataGridViewTextBoxColumn25->Name = L"dataGridViewTextBoxColumn25";
		this->dataGridViewTextBoxColumn25->ReadOnly = true;
		this->dataGridViewTextBoxColumn25->Resizable = System::Windows::Forms::DataGridViewTriState::False;
		this->dataGridViewTextBoxColumn25->SortMode = System::Windows::Forms::DataGridViewColumnSortMode::NotSortable;
		this->dataGridViewTextBoxColumn25->Width = 70;
		// 
		// dataGridViewTextBoxColumn26
		// 
		this->dataGridViewTextBoxColumn26->AutoSizeMode = System::Windows::Forms::DataGridViewAutoSizeColumnMode::None;
		dataGridViewCellStyle32->Alignment = System::Windows::Forms::DataGridViewContentAlignment::MiddleCenter;
		dataGridViewCellStyle32->WrapMode = System::Windows::Forms::DataGridViewTriState::True;
		this->dataGridViewTextBoxColumn26->DefaultCellStyle = dataGridViewCellStyle32;
		this->dataGridViewTextBoxColumn26->HeaderText = L"11 ... 9";
		this->dataGridViewTextBoxColumn26->Name = L"dataGridViewTextBoxColumn26";
		this->dataGridViewTextBoxColumn26->ReadOnly = true;
		this->dataGridViewTextBoxColumn26->Resizable = System::Windows::Forms::DataGridViewTriState::False;
		this->dataGridViewTextBoxColumn26->SortMode = System::Windows::Forms::DataGridViewColumnSortMode::NotSortable;
		this->dataGridViewTextBoxColumn26->Width = 46;
		// 
		// Column18
		// 
		this->Column18->AutoSizeMode = System::Windows::Forms::DataGridViewAutoSizeColumnMode::None;
		dataGridViewCellStyle33->Alignment = System::Windows::Forms::DataGridViewContentAlignment::MiddleCenter;
		this->Column18->DefaultCellStyle = dataGridViewCellStyle33;
		this->Column18->HeaderText = L"8";
		this->Column18->Name = L"Column18";
		this->Column18->ReadOnly = true;
		this->Column18->Resizable = System::Windows::Forms::DataGridViewTriState::False;
		this->Column18->SortMode = System::Windows::Forms::DataGridViewColumnSortMode::NotSortable;
		this->Column18->Width = 19;
		// 
		// dataGridViewTextBoxColumn27
		// 
		this->dataGridViewTextBoxColumn27->AutoSizeMode = System::Windows::Forms::DataGridViewAutoSizeColumnMode::None;
		dataGridViewCellStyle34->Alignment = System::Windows::Forms::DataGridViewContentAlignment::MiddleCenter;
		dataGridViewCellStyle34->WrapMode = System::Windows::Forms::DataGridViewTriState::True;
		this->dataGridViewTextBoxColumn27->DefaultCellStyle = dataGridViewCellStyle34;
		this->dataGridViewTextBoxColumn27->HeaderText = L"7";
		this->dataGridViewTextBoxColumn27->Name = L"dataGridViewTextBoxColumn27";
		this->dataGridViewTextBoxColumn27->ReadOnly = true;
		this->dataGridViewTextBoxColumn27->Resizable = System::Windows::Forms::DataGridViewTriState::False;
		this->dataGridViewTextBoxColumn27->SortMode = System::Windows::Forms::DataGridViewColumnSortMode::NotSortable;
		this->dataGridViewTextBoxColumn27->Width = 19;
		// 
		// dataGridViewTextBoxColumn28
		// 
		this->dataGridViewTextBoxColumn28->AutoSizeMode = System::Windows::Forms::DataGridViewAutoSizeColumnMode::None;
		dataGridViewCellStyle35->Alignment = System::Windows::Forms::DataGridViewContentAlignment::MiddleCenter;
		dataGridViewCellStyle35->WrapMode = System::Windows::Forms::DataGridViewTriState::True;
		this->dataGridViewTextBoxColumn28->DefaultCellStyle = dataGridViewCellStyle35;
		this->dataGridViewTextBoxColumn28->HeaderText = L"6";
		this->dataGridViewTextBoxColumn28->Name = L"dataGridViewTextBoxColumn28";
		this->dataGridViewTextBoxColumn28->ReadOnly = true;
		this->dataGridViewTextBoxColumn28->Resizable = System::Windows::Forms::DataGridViewTriState::False;
		this->dataGridViewTextBoxColumn28->SortMode = System::Windows::Forms::DataGridViewColumnSortMode::NotSortable;
		this->dataGridViewTextBoxColumn28->Width = 25;
		// 
		// dataGridViewTextBoxColumn29
		// 
		this->dataGridViewTextBoxColumn29->AutoSizeMode = System::Windows::Forms::DataGridViewAutoSizeColumnMode::None;
		dataGridViewCellStyle36->Alignment = System::Windows::Forms::DataGridViewContentAlignment::MiddleCenter;
		dataGridViewCellStyle36->WrapMode = System::Windows::Forms::DataGridViewTriState::True;
		this->dataGridViewTextBoxColumn29->DefaultCellStyle = dataGridViewCellStyle36;
		this->dataGridViewTextBoxColumn29->HeaderText = L"5";
		this->dataGridViewTextBoxColumn29->Name = L"dataGridViewTextBoxColumn29";
		this->dataGridViewTextBoxColumn29->ReadOnly = true;
		this->dataGridViewTextBoxColumn29->Resizable = System::Windows::Forms::DataGridViewTriState::False;
		this->dataGridViewTextBoxColumn29->SortMode = System::Windows::Forms::DataGridViewColumnSortMode::NotSortable;
		this->dataGridViewTextBoxColumn29->Width = 19;
		// 
		// dataGridViewTextBoxColumn30
		// 
		this->dataGridViewTextBoxColumn30->AutoSizeMode = System::Windows::Forms::DataGridViewAutoSizeColumnMode::None;
		dataGridViewCellStyle37->Alignment = System::Windows::Forms::DataGridViewContentAlignment::MiddleCenter;
		dataGridViewCellStyle37->WrapMode = System::Windows::Forms::DataGridViewTriState::True;
		this->dataGridViewTextBoxColumn30->DefaultCellStyle = dataGridViewCellStyle37;
		this->dataGridViewTextBoxColumn30->HeaderText = L"4";
		this->dataGridViewTextBoxColumn30->Name = L"dataGridViewTextBoxColumn30";
		this->dataGridViewTextBoxColumn30->ReadOnly = true;
		this->dataGridViewTextBoxColumn30->Resizable = System::Windows::Forms::DataGridViewTriState::False;
		this->dataGridViewTextBoxColumn30->SortMode = System::Windows::Forms::DataGridViewColumnSortMode::NotSortable;
		this->dataGridViewTextBoxColumn30->Width = 19;
		// 
		// dataGridViewTextBoxColumn31
		// 
		this->dataGridViewTextBoxColumn31->AutoSizeMode = System::Windows::Forms::DataGridViewAutoSizeColumnMode::None;
		dataGridViewCellStyle38->Alignment = System::Windows::Forms::DataGridViewContentAlignment::MiddleCenter;
		dataGridViewCellStyle38->WrapMode = System::Windows::Forms::DataGridViewTriState::True;
		this->dataGridViewTextBoxColumn31->DefaultCellStyle = dataGridViewCellStyle38;
		this->dataGridViewTextBoxColumn31->HeaderText = L"3";
		this->dataGridViewTextBoxColumn31->Name = L"dataGridViewTextBoxColumn31";
		this->dataGridViewTextBoxColumn31->ReadOnly = true;
		this->dataGridViewTextBoxColumn31->Resizable = System::Windows::Forms::DataGridViewTriState::False;
		this->dataGridViewTextBoxColumn31->SortMode = System::Windows::Forms::DataGridViewColumnSortMode::NotSortable;
		this->dataGridViewTextBoxColumn31->Width = 19;
		// 
		// dataGridViewTextBoxColumn32
		// 
		this->dataGridViewTextBoxColumn32->AutoSizeMode = System::Windows::Forms::DataGridViewAutoSizeColumnMode::None;
		dataGridViewCellStyle39->Alignment = System::Windows::Forms::DataGridViewContentAlignment::MiddleCenter;
		dataGridViewCellStyle39->WrapMode = System::Windows::Forms::DataGridViewTriState::True;
		this->dataGridViewTextBoxColumn32->DefaultCellStyle = dataGridViewCellStyle39;
		this->dataGridViewTextBoxColumn32->HeaderText = L"2";
		this->dataGridViewTextBoxColumn32->Name = L"dataGridViewTextBoxColumn32";
		this->dataGridViewTextBoxColumn32->ReadOnly = true;
		this->dataGridViewTextBoxColumn32->Resizable = System::Windows::Forms::DataGridViewTriState::False;
		this->dataGridViewTextBoxColumn32->SortMode = System::Windows::Forms::DataGridViewColumnSortMode::NotSortable;
		this->dataGridViewTextBoxColumn32->Width = 19;
		// 
		// dataGridViewTextBoxColumn33
		// 
		this->dataGridViewTextBoxColumn33->AutoSizeMode = System::Windows::Forms::DataGridViewAutoSizeColumnMode::None;
		dataGridViewCellStyle40->Alignment = System::Windows::Forms::DataGridViewContentAlignment::MiddleCenter;
		dataGridViewCellStyle40->WrapMode = System::Windows::Forms::DataGridViewTriState::True;
		this->dataGridViewTextBoxColumn33->DefaultCellStyle = dataGridViewCellStyle40;
		this->dataGridViewTextBoxColumn33->HeaderText = L"1";
		this->dataGridViewTextBoxColumn33->Name = L"dataGridViewTextBoxColumn33";
		this->dataGridViewTextBoxColumn33->ReadOnly = true;
		this->dataGridViewTextBoxColumn33->Resizable = System::Windows::Forms::DataGridViewTriState::False;
		this->dataGridViewTextBoxColumn33->SortMode = System::Windows::Forms::DataGridViewColumnSortMode::NotSortable;
		this->dataGridViewTextBoxColumn33->Width = 19;
		// 
		// dataGridViewTextBoxColumn34
		// 
		this->dataGridViewTextBoxColumn34->AutoSizeMode = System::Windows::Forms::DataGridViewAutoSizeColumnMode::Fill;
		dataGridViewCellStyle41->Alignment = System::Windows::Forms::DataGridViewContentAlignment::MiddleCenter;
		dataGridViewCellStyle41->WrapMode = System::Windows::Forms::DataGridViewTriState::True;
		this->dataGridViewTextBoxColumn34->DefaultCellStyle = dataGridViewCellStyle41;
		this->dataGridViewTextBoxColumn34->HeaderText = L"0";
		this->dataGridViewTextBoxColumn34->Name = L"dataGridViewTextBoxColumn34";
		this->dataGridViewTextBoxColumn34->ReadOnly = true;
		this->dataGridViewTextBoxColumn34->Resizable = System::Windows::Forms::DataGridViewTriState::False;
		this->dataGridViewTextBoxColumn34->SortMode = System::Windows::Forms::DataGridViewColumnSortMode::NotSortable;
		// 
		// dgvProperties
		// 
		this->dgvProperties->AllowUserToAddRows = false;
		this->dgvProperties->AllowUserToDeleteRows = false;
		this->dgvProperties->AllowUserToResizeColumns = false;
		this->dgvProperties->AllowUserToResizeRows = false;
		this->dgvProperties->AutoSizeRowsMode = System::Windows::Forms::DataGridViewAutoSizeRowsMode::AllCells;
		this->dgvProperties->ColumnHeadersHeightSizeMode = System::Windows::Forms::DataGridViewColumnHeadersHeightSizeMode::AutoSize;
		this->dgvProperties->Columns->AddRange(gcnew cli::array< System::Windows::Forms::DataGridViewColumn^  >(14) {
			this->Column14,
				this->Column1, this->Column2, this->Column3, this->Column4, this->Column5, this->Column6, this->Column7, this->Column8, this->Column9,
				this->Column10, this->Column11, this->Column12, this->Column13
		});
		this->dgvProperties->Enabled = false;
		this->dgvProperties->Location = System::Drawing::Point(44, 3);
		this->dgvProperties->MultiSelect = false;
		this->dgvProperties->Name = L"dgvProperties";
		this->dgvProperties->ReadOnly = true;
		this->dgvProperties->RowHeadersVisible = false;
		this->dgvProperties->RowHeadersWidthSizeMode = System::Windows::Forms::DataGridViewRowHeadersWidthSizeMode::DisableResizing;
		this->dgvProperties->SelectionMode = System::Windows::Forms::DataGridViewSelectionMode::CellSelect;
		this->dgvProperties->Size = System::Drawing::Size(455, 158);
		this->dgvProperties->TabIndex = 30;
		// 
		// Column14
		// 
		this->Column14->AutoSizeMode = System::Windows::Forms::DataGridViewAutoSizeColumnMode::None;
		dataGridViewCellStyle42->Alignment = System::Windows::Forms::DataGridViewContentAlignment::MiddleLeft;
		this->Column14->DefaultCellStyle = dataGridViewCellStyle42;
		this->Column14->HeaderText = L"Table";
		this->Column14->Name = L"Column14";
		this->Column14->ReadOnly = true;
		this->Column14->Resizable = System::Windows::Forms::DataGridViewTriState::False;
		this->Column14->SortMode = System::Windows::Forms::DataGridViewColumnSortMode::NotSortable;
		this->Column14->Width = 40;
		// 
		// Column1
		// 
		this->Column1->AutoSizeMode = System::Windows::Forms::DataGridViewAutoSizeColumnMode::None;
		dataGridViewCellStyle43->Alignment = System::Windows::Forms::DataGridViewContentAlignment::MiddleCenter;
		dataGridViewCellStyle43->WrapMode = System::Windows::Forms::DataGridViewTriState::True;
		this->Column1->DefaultCellStyle = dataGridViewCellStyle43;
		this->Column1->HeaderText = L"63";
		this->Column1->Name = L"Column1";
		this->Column1->ReadOnly = true;
		this->Column1->Resizable = System::Windows::Forms::DataGridViewTriState::False;
		this->Column1->SortMode = System::Windows::Forms::DataGridViewColumnSortMode::NotSortable;
		this->Column1->Width = 25;
		// 
		// Column2
		// 
		this->Column2->AutoSizeMode = System::Windows::Forms::DataGridViewAutoSizeColumnMode::None;
		dataGridViewCellStyle44->Alignment = System::Windows::Forms::DataGridViewContentAlignment::MiddleCenter;
		dataGridViewCellStyle44->WrapMode = System::Windows::Forms::DataGridViewTriState::True;
		this->Column2->DefaultCellStyle = dataGridViewCellStyle44;
		this->Column2->HeaderText = L"62 ... 52";
		this->Column2->Name = L"Column2";
		this->Column2->ReadOnly = true;
		this->Column2->Resizable = System::Windows::Forms::DataGridViewTriState::False;
		this->Column2->SortMode = System::Windows::Forms::DataGridViewColumnSortMode::NotSortable;
		this->Column2->Width = 52;
		// 
		// Column3
		// 
		this->Column3->AutoSizeMode = System::Windows::Forms::DataGridViewAutoSizeColumnMode::None;
		dataGridViewCellStyle45->Alignment = System::Windows::Forms::DataGridViewContentAlignment::MiddleCenter;
		dataGridViewCellStyle45->WrapMode = System::Windows::Forms::DataGridViewTriState::True;
		this->Column3->DefaultCellStyle = dataGridViewCellStyle45;
		this->Column3->HeaderText = L"51 ... M";
		this->Column3->Name = L"Column3";
		this->Column3->ReadOnly = true;
		this->Column3->Resizable = System::Windows::Forms::DataGridViewTriState::False;
		this->Column3->SortMode = System::Windows::Forms::DataGridViewColumnSortMode::NotSortable;
		this->Column3->Width = 55;
		// 
		// Column4
		// 
		this->Column4->AutoSizeMode = System::Windows::Forms::DataGridViewAutoSizeColumnMode::None;
		dataGridViewCellStyle46->Alignment = System::Windows::Forms::DataGridViewContentAlignment::MiddleCenter;
		dataGridViewCellStyle46->WrapMode = System::Windows::Forms::DataGridViewTriState::True;
		this->Column4->DefaultCellStyle = dataGridViewCellStyle46;
		this->Column4->HeaderText = L"M-1 ... 12";
		this->Column4->Name = L"Column4";
		this->Column4->ReadOnly = true;
		this->Column4->Resizable = System::Windows::Forms::DataGridViewTriState::False;
		this->Column4->SortMode = System::Windows::Forms::DataGridViewColumnSortMode::NotSortable;
		this->Column4->Width = 70;
		// 
		// Column5
		// 
		this->Column5->AutoSizeMode = System::Windows::Forms::DataGridViewAutoSizeColumnMode::None;
		dataGridViewCellStyle47->Alignment = System::Windows::Forms::DataGridViewContentAlignment::MiddleCenter;
		dataGridViewCellStyle47->WrapMode = System::Windows::Forms::DataGridViewTriState::True;
		this->Column5->DefaultCellStyle = dataGridViewCellStyle47;
		this->Column5->HeaderText = L"11 ... 8";
		this->Column5->Name = L"Column5";
		this->Column5->ReadOnly = true;
		this->Column5->Resizable = System::Windows::Forms::DataGridViewTriState::False;
		this->Column5->SortMode = System::Windows::Forms::DataGridViewColumnSortMode::NotSortable;
		this->Column5->Width = 46;
		// 
		// Column6
		// 
		this->Column6->AutoSizeMode = System::Windows::Forms::DataGridViewAutoSizeColumnMode::None;
		dataGridViewCellStyle48->Alignment = System::Windows::Forms::DataGridViewContentAlignment::MiddleCenter;
		dataGridViewCellStyle48->WrapMode = System::Windows::Forms::DataGridViewTriState::True;
		this->Column6->DefaultCellStyle = dataGridViewCellStyle48;
		this->Column6->HeaderText = L"7";
		this->Column6->Name = L"Column6";
		this->Column6->ReadOnly = true;
		this->Column6->Resizable = System::Windows::Forms::DataGridViewTriState::False;
		this->Column6->SortMode = System::Windows::Forms::DataGridViewColumnSortMode::NotSortable;
		this->Column6->Width = 19;
		// 
		// Column7
		// 
		this->Column7->AutoSizeMode = System::Windows::Forms::DataGridViewAutoSizeColumnMode::None;
		dataGridViewCellStyle49->Alignment = System::Windows::Forms::DataGridViewContentAlignment::MiddleCenter;
		dataGridViewCellStyle49->WrapMode = System::Windows::Forms::DataGridViewTriState::True;
		this->Column7->DefaultCellStyle = dataGridViewCellStyle49;
		this->Column7->HeaderText = L"6";
		this->Column7->Name = L"Column7";
		this->Column7->ReadOnly = true;
		this->Column7->Resizable = System::Windows::Forms::DataGridViewTriState::False;
		this->Column7->SortMode = System::Windows::Forms::DataGridViewColumnSortMode::NotSortable;
		this->Column7->Width = 25;
		// 
		// Column8
		// 
		this->Column8->AutoSizeMode = System::Windows::Forms::DataGridViewAutoSizeColumnMode::None;
		dataGridViewCellStyle50->Alignment = System::Windows::Forms::DataGridViewContentAlignment::MiddleCenter;
		dataGridViewCellStyle50->WrapMode = System::Windows::Forms::DataGridViewTriState::True;
		this->Column8->DefaultCellStyle = dataGridViewCellStyle50;
		this->Column8->HeaderText = L"5";
		this->Column8->Name = L"Column8";
		this->Column8->ReadOnly = true;
		this->Column8->Resizable = System::Windows::Forms::DataGridViewTriState::False;
		this->Column8->SortMode = System::Windows::Forms::DataGridViewColumnSortMode::NotSortable;
		this->Column8->Width = 19;
		// 
		// Column9
		// 
		this->Column9->AutoSizeMode = System::Windows::Forms::DataGridViewAutoSizeColumnMode::None;
		dataGridViewCellStyle51->Alignment = System::Windows::Forms::DataGridViewContentAlignment::MiddleCenter;
		dataGridViewCellStyle51->WrapMode = System::Windows::Forms::DataGridViewTriState::True;
		this->Column9->DefaultCellStyle = dataGridViewCellStyle51;
		this->Column9->HeaderText = L"4";
		this->Column9->Name = L"Column9";
		this->Column9->ReadOnly = true;
		this->Column9->Resizable = System::Windows::Forms::DataGridViewTriState::False;
		this->Column9->SortMode = System::Windows::Forms::DataGridViewColumnSortMode::NotSortable;
		this->Column9->Width = 19;
		// 
		// Column10
		// 
		this->Column10->AutoSizeMode = System::Windows::Forms::DataGridViewAutoSizeColumnMode::None;
		dataGridViewCellStyle52->Alignment = System::Windows::Forms::DataGridViewContentAlignment::MiddleCenter;
		dataGridViewCellStyle52->WrapMode = System::Windows::Forms::DataGridViewTriState::True;
		this->Column10->DefaultCellStyle = dataGridViewCellStyle52;
		this->Column10->HeaderText = L"3";
		this->Column10->Name = L"Column10";
		this->Column10->ReadOnly = true;
		this->Column10->Resizable = System::Windows::Forms::DataGridViewTriState::False;
		this->Column10->SortMode = System::Windows::Forms::DataGridViewColumnSortMode::NotSortable;
		this->Column10->Width = 19;
		// 
		// Column11
		// 
		this->Column11->AutoSizeMode = System::Windows::Forms::DataGridViewAutoSizeColumnMode::None;
		dataGridViewCellStyle53->Alignment = System::Windows::Forms::DataGridViewContentAlignment::MiddleCenter;
		dataGridViewCellStyle53->WrapMode = System::Windows::Forms::DataGridViewTriState::True;
		this->Column11->DefaultCellStyle = dataGridViewCellStyle53;
		this->Column11->HeaderText = L"2";
		this->Column11->Name = L"Column11";
		this->Column11->ReadOnly = true;
		this->Column11->Resizable = System::Windows::Forms::DataGridViewTriState::False;
		this->Column11->SortMode = System::Windows::Forms::DataGridViewColumnSortMode::NotSortable;
		this->Column11->Width = 19;
		// 
		// Column12
		// 
		this->Column12->AutoSizeMode = System::Windows::Forms::DataGridViewAutoSizeColumnMode::None;
		dataGridViewCellStyle54->Alignment = System::Windows::Forms::DataGridViewContentAlignment::MiddleCenter;
		dataGridViewCellStyle54->WrapMode = System::Windows::Forms::DataGridViewTriState::True;
		this->Column12->DefaultCellStyle = dataGridViewCellStyle54;
		this->Column12->HeaderText = L"1";
		this->Column12->Name = L"Column12";
		this->Column12->ReadOnly = true;
		this->Column12->Resizable = System::Windows::Forms::DataGridViewTriState::False;
		this->Column12->SortMode = System::Windows::Forms::DataGridViewColumnSortMode::NotSortable;
		this->Column12->Width = 19;
		// 
		// Column13
		// 
		this->Column13->AutoSizeMode = System::Windows::Forms::DataGridViewAutoSizeColumnMode::Fill;
		dataGridViewCellStyle55->Alignment = System::Windows::Forms::DataGridViewContentAlignment::MiddleCenter;
		dataGridViewCellStyle55->WrapMode = System::Windows::Forms::DataGridViewTriState::True;
		this->Column13->DefaultCellStyle = dataGridViewCellStyle55;
		this->Column13->HeaderText = L"0";
		this->Column13->Name = L"Column13";
		this->Column13->ReadOnly = true;
		this->Column13->Resizable = System::Windows::Forms::DataGridViewTriState::False;
		this->Column13->SortMode = System::Windows::Forms::DataGridViewColumnSortMode::NotSortable;
		// 
		// PropertiesTabs
		// 
		this->PropertiesTabs->Controls->Add(this->TablesTab);
		this->PropertiesTabs->Controls->Add(this->RegistersTab);
		this->PropertiesTabs->Controls->Add(this->AddressTab);
		this->PropertiesTabs->Location = System::Drawing::Point(515, 709);
		this->PropertiesTabs->Name = L"PropertiesTabs";
		this->PropertiesTabs->SelectedIndex = 0;
		this->PropertiesTabs->Size = System::Drawing::Size(840, 291);
		this->PropertiesTabs->TabIndex = 32;
		// 
		// MainForm
		// 
		this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
		this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
		this->ClientSize = System::Drawing::Size(1362, 1025);
		this->Controls->Add(this->PropertiesTabs);
		this->Controls->Add(this->panel1);
		this->Controls->Add(this->label1);
		this->Controls->Add(this->dgvMemory);
		this->Controls->Add(this->dgvProcesses);
		this->Controls->Add(this->statusStrip);
		this->Controls->Add(this->menuStrip1);
		this->Controls->Add(this->Refresh_button);
		this->MainMenuStrip = this->menuStrip1;
		this->Name = L"MainForm";
		this->Load += gcnew System::EventHandler(this, &MainForm::MainForm_Load);
		this->panel1->ResumeLayout(false);
		this->panel1->PerformLayout();
		(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->dataGridViewPT))->EndInit();
		(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->dataGridViewPDE))->EndInit();
		(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->dataGridViewPDP))->EndInit();
		(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->dataGridViewPML4))->EndInit();
		this->statusStrip->ResumeLayout(false);
		this->statusStrip->PerformLayout();
		(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->dgvProcesses))->EndInit();
		(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->dgvMemory))->EndInit();
		this->menuStrip1->ResumeLayout(false);
		this->menuStrip1->PerformLayout();
		this->AddressTab->ResumeLayout(false);
		this->RegistersTab->ResumeLayout(false);
		this->RegistersTab->PerformLayout();
		(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->dataGridCR3))->EndInit();
		(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->dataGridCR0))->EndInit();
		this->TablesTab->ResumeLayout(false);
		(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->dgvPTProperties))->EndInit();
		(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->dgvProperties))->EndInit();
		this->PropertiesTabs->ResumeLayout(false);
		this->ResumeLayout(false);
		this->PerformLayout();

	}
#pragma warning(push)
#pragma warning(disable: 4965)
	void InitializeCustomComponents(void)
	{
		if (Utils::IsElevated())
		{
			this->Text = L"Page Table Explorer (Administrator)";
		}
		else
		{
			this->Text = L"Page Table Explorer";
		}
		this->PTs[TableType::PML4] = this->dataGridViewPML4;
		this->PTs[TableType::PDP] = this->dataGridViewPDP;
		this->PTs[TableType::PD] = this->dataGridViewPDE;
		this->PTs[TableType::PT] = this->dataGridViewPT;

		this->decIndexes[0] = this->labelDec47_39;
		this->decIndexes[1] = this->labelDec38_30;
		this->decIndexes[2] = this->labelDec29_21;
		this->decIndexes[3] = this->labelDec20_12;
		this->decIndexes[4] = this->labelDec11_00;


		this->dgvProperties->Rows->Add("", "X\nD", "AVL", "Reserved\n(0)", "Bits 12 - (M-1)\r\nof address", "AVL", "R\nS\nV\nD\n0", "AVL", "A", "P\nC\nD", "P\nW\nT", "U\n/\nS", "R\n/\nW","P");
		this->dgvProperties->Rows->Add("PML4", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
		this->dgvProperties->Rows->Add("PDP", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
		this->dgvProperties->Rows->Add("PDE", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);

		this->dgvProperties->Rows[0]->DefaultCellStyle->BackColor = System::Drawing::SystemColors::ControlLight;
		this->dgvProperties->CurrentCell = nullptr;

		this->dgvPTProperties->Rows->Add("", "X\nD", "P\nK",  "A\nV\nL", "Reserved\n(0)", "Bits 12 - (M-1)\r\nof address", "A\nV\nL", "G", "P\nA\nT", "D", "A", "P\nC\nD", "P\nW\nT", "U\n/\nS", "R\n/\nW", "P");
		this->dgvPTProperties->Rows->Add("PTE", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
		this->dgvPTProperties->Rows[0]->DefaultCellStyle->BackColor = System::Drawing::SystemColors::ControlLight;

		this->dgvProperties->CurrentCell = nullptr;

		this->dataGridCR0->Rows->Add("", "Reserved\n(0)", "P\nG", "C\nD", "N\nW", "Reserved\n(0)", "A\nM", "0", "W\nP", "Reserved\n(0)", "N\nE", "E\nT", "T\nS", "E\nM", "M\nP", "P\nE");
		this->dataGridCR0->Rows->Add("CR0", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
		this->dataGridCR0->CurrentCell = nullptr;

		this->dataGridCR3->Rows->Add("", "Reserved (0)", "Phys", "Reserved (0)", "PCD", "PWT", "Reserved (0)");
		this->dataGridCR3->Rows->Add("CR3", 0, 0, 0, 0, 0, 0);
		this->dataGridCR3->CurrentCell = nullptr;

		dgvMemory->AutoGenerateColumns = false;
		dgvMemory->Columns["BaseAddress"]->DataPropertyName = "BaseAddress";
		dgvMemory->Columns["Protection"]->DataPropertyName = "Protection";

		dgvProcesses->AutoGenerateColumns = false;
		dgvProcesses->Columns["PID"]->DataPropertyName = "PID";
		dgvProcesses->Columns["ProcessName"]->DataPropertyName = "ProcessName";			
			
		this->dataGridViewPML4->AutoGenerateColumns = false;
		this->dataGridViewPDP->AutoGenerateColumns = false;
		this->dataGridViewPDE->AutoGenerateColumns = false;
		this->dataGridViewPT->AutoGenerateColumns = false;

		this->dataGridViewPML4->Columns["Index"]->DataPropertyName = "Index";
		this->dataGridViewPML4->Columns["Value"]->DataPropertyName = "Value";
		this->dataGridViewPDP->Columns["Index2"]->DataPropertyName = "Index";
		this->dataGridViewPDP->Columns["Value2"]->DataPropertyName = "Value";
		this->dataGridViewPDE->Columns["Index3"]->DataPropertyName = "Index";
		this->dataGridViewPDE->Columns["Value3"]->DataPropertyName = "Value";
		this->dataGridViewPT->Columns["Index4"]->DataPropertyName = "Index";
		this->dataGridViewPT->Columns["Value4"]->DataPropertyName = "Value";

		SetFormIcon();
	}
#pragma warning(pop)

#pragma endregion
private: System::Void MainForm_Load(System::Object^ sender, System::EventArgs^ e) {
}
public: System::Void RefreshButton_Click(System::Object^ sender, System::EventArgs^ e);
private: System::Void DrawAddressTable(System::Object^ sender, System::Windows::Forms::DataGridViewCellEventArgs^ e);
private: System::Void DrawPT(TableType type, UInt16 value, void* physicalAddress);
private: System::Void DrawLine(Graphics^ g, Point from, Point to, bool isArrow);
private: System::Void DgvProcesses_CellContentClick(System::Object^ sender, System::Windows::Forms::DataGridViewCellEventArgs^ e);
public: System::Void DgvMemory_CellContentClick(System::Object^ sender, System::Windows::Forms::DataGridViewCellEventArgs^ e);
private: System::Void aboutToolStripMenuItem1_Click(System::Object^ sender, System::EventArgs^ e)
{
	System::String^ about = L"         Page Table Explorer v1.0\n\n" \
				            L"             Dmitry Podvigalkin\n\n" \
				            L"                     2025\n";
	MessageBox::Show(about);
}
private: System::Void RefreshToolStripMenuItem_Click(System::Object^ sender, System::EventArgs^ e)
{
	EnumProcesses();
}
private: System::Void exitToolStripMenuItem_Click(System::Object^ sender, System::EventArgs^ e)
{
	this->Close();
}
private: System::Void ButtonUnpage_Click(System::Object^ sender, System::EventArgs^ e);
private: System::Void DataGridViewPDP_CellClick(System::Object^ sender, System::Windows::Forms::DataGridViewCellEventArgs^ e);
private: System::Void DataGridViewPT_CellClick(System::Object^ sender, System::Windows::Forms::DataGridViewCellEventArgs^ e);
private: System::Void DataGridViewPDE_CellClick(System::Object^ sender, System::Windows::Forms::DataGridViewCellEventArgs^ e);
private: System::Void DataGridViewPML4_CellClick(System::Object^ sender, System::Windows::Forms::DataGridViewCellEventArgs^ e);
private: System::Void SetFormIcon();
private: System::Void EnumProcesses();
private: System::Void ForceCurrentPageToPhysicalMemory();
private: System::Void PTCellClick(System::Object^ sender, System::Windows::Forms::DataGridViewCellEventArgs^ e, const TableType type);
private: System::Void UpdateUIBasedOnAddress(unsigned long long address, void* driverResponse);
private: System::Void DrawArrows(TableType Type);
public: bool IsDebugPrivileged()
{
	return s_bAreWeDebugPrivileged;
};

public: void SetDebugPrivileged()
{
	s_bAreWeDebugPrivileged = true;
};

public: UInt64 GetCustomAddress()
{
	return m_customAddress;
};

private: bool s_bAreWeDebugPrivileged{ false };
private: bool s_UnpageButtonPressed{ false };
private: UInt64 m_customAddress{ 0 };
}; //class MainForm

} //namespace PTE
