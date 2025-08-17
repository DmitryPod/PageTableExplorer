# PageTableExplorer
#### _Virtual to physical addresses memory visualizer_

The tool gives visual representation of virtaual and physical memory in Windows OS.

![Alt text](screenshot.png?raw=true "Screenshot")

## Requirments
Since the driver is test-signed, the tool requires Windows to be runnign with the Test Mode enabled.
To enable it run following command and restart Windows:
```cmd
bcdedit /set TESTSIGNING ON
```
## Running the tool

Copy files from build directory and run PageTableExplorer.exe

## Features

- The content of a page view
- "Page walking"
- "Unpaging" the memory

## Building for source

Open PageTableExplorer.sln in Visual Studio and build the solution.
In case of "SignTool Error: No certificates were found that met all the given criteria." error,
reopen VIsual Studio as Administorator.
The tool was written and tested with Visual Studio 2022.

#### Limitations

x86 is not supported at this point.

## License

MIT
