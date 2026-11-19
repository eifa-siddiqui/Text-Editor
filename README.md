# Text Editor

A C++ text editor with a console interface and a browser interface.

## Requirements

- Windows
- MinGW g++
- A web browser

## Run the Executables

### Console editor

```powershell
.\TextEditor.exe
```

### Browser editor

1. Start the C++ server:

```powershell
.\TextEditorServer.exe
```

2. Open `ui\index.html` in a browser.
3. Keep the server running while using the browser.

The browser communicates with the C++ server at `http://127.0.0.1:8080`.

## Compile from Source

From the project root:

### Console editor

```powershell
g++ -std=c++17 -Wall -Wextra -pedantic -Iinclude src\main.cpp src\app\ConsoleApp.cpp src\algorithms\Search.cpp src\algorithms\Replace.cpp src\core\Document.cpp src\data_structures\HashTable.cpp src\data_structures\LinkedList.cpp src\data_structures\Stack.cpp src\file_handling\FileManager.cpp src\pdf\PDFExporter.cpp src\undo\UndoManager.cpp -o TextEditor.exe
```

### C++ browser server

```powershell
g++ -std=c++17 -Wall -Wextra -pedantic -Iinclude src\server\TextEditorServer.cpp src\core\Document.cpp src\algorithms\Search.cpp src\algorithms\Replace.cpp src\data_structures\HashTable.cpp src\data_structures\LinkedList.cpp src\data_structures\Stack.cpp src\file_handling\FileManager.cpp src\pdf\PDFExporter.cpp src\undo\UndoManager.cpp -lws2_32 -lcomdlg32 -o TextEditorServer.exe
```

## Project Structure

```text
Text-Editor/
|-- include/
|   |-- algorithms/
|   |   |-- Replace.h
|   |   `-- Search.h
|   |-- app/
|   |   `-- ConsoleApp.h
|   |-- core/
|   |   `-- Document.h
|   |-- data_structures/
|   |   |-- HashTable.h
|   |   |-- LinkedList.h
|   |   `-- Stack.h
|   |-- file_handling/
|   |   `-- FileManager.h
|   |-- pdf/
|   |   `-- PDFExporter.h
|   `-- undo/
|       `-- UndoManager.h
|-- src/
|   |-- algorithms/
|   |   |-- Replace.cpp
|   |   `-- Search.cpp
|   |-- app/
|   |   `-- ConsoleApp.cpp
|   |-- core/
|   |   `-- Document.cpp
|   |-- data_structures/
|   |   |-- HashTable.cpp
|   |   |-- LinkedList.cpp
|   |   `-- Stack.cpp
|   |-- file_handling/
|   |   `-- FileManager.cpp
|   |-- pdf/
|   |   `-- PDFExporter.cpp
|   |-- server/
|   |   `-- TextEditorServer.cpp
|   |-- undo/
|   |   `-- UndoManager.cpp
|   `-- main.cpp
|-- ui/
|   `-- index.html
|-- TextEditor.exe
|-- TextEditorServer.exe
`-- README.md
```
