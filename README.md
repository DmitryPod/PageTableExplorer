# PageTableExplorer
#### _Virtual to Physical Address Memory Visualizer_

Page Table Explorer is a Windows tool that provides a visual representation of virtual and physical memory mappings, helping researchers, students, and developers explore how memory is managed at the page-table level.

![Alt text](screenshot.png?raw=true "Screenshot")

## Features
- 🖼 Page visualization – inspect the contents of memory pages
- 🔎 Page walking – traverse page tables step by step
- 📦 Unpaging support – analyze swapped-out memory
- 🛠 Designed for low-level debugging, reverse engineering, and OS internals learning

## Requirments
Since the driver is test-signed, Windows must be run in Test Mode.
Enable Test Mode by opening a Command Prompt as Administrator and running the following command (Windows reboot required):
```cmd
bcdedit /set TESTSIGNING ON
```

## Running the tool

1. Copy the release files to your target machine.
2. Run PageTableExplorer.exe.
3. Start exploring memory mappings interactively.

## Building for source
1. Open PageTableExplorer.sln in Visual Studio 2022.
2. Build the solution (Ctrl+Shift+B).
3. If you encounter the folloing error, reopen Visual Studio as Administrator:
```cmd
SignTool Error: No certificates were found that met all the given criteria
```
###### 📌 Tested with Visual Studio 2022 on Windows 10/11.
###### ❌ Currently, x86 is not supported (x64 only).
______________________
## License
MIT
This project is licensed under the MIT License.
