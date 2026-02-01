# Getting Started – Installation and Build

This section guides you through setting up the **spispectrumplay** project for Windows. You’ll install the required audio libraries, configure your build environment, compile the executable, and prepare runtime files for playback.

## 1. Install BASS and BASSASIO 🔊

Before building, obtain Un4seen’s audio libraries for Windows.

- **Download BASS** and **BASSASIO** from their official distributors.
- Copy the headers and import libraries:
- `bass.h`
- `bassasio.h`
- `bass.lib` (or `libbass.a`)
- `bassasio.lib` (or `libbassasio.a`)
- Place the headers where your compiler can find them (e.g. in `include\bass`).
- Place the import libraries in your linker path (e.g. in `lib\bass`).
- Ensure the **DLLs** are available at runtime alongside your EXE or on `PATH`:
- `BASS.DLL`
- `BASSASIO.DLL`

## 2. Set Up the Build Environment 🔧

> **Why?** BASS provides core audio playback, while BASSASIO adds low-latency ASIO support. Missing or mismatched DLLs will prevent the application from starting.

Configure a Windows C/C++ toolchain with `make` support and link against Win32 GUI libraries.

- Install a toolchain such as **MinGW** or **MSYS2** with:
- `gcc`/`g++`
- `make`
- Confirm the **Windows SDK** libraries are available:
- `comdlg32` (common dialogs)
- `gdi32` (graphics drawing)
- `winmm` (multimedia timers)
- Adjust include paths in your IDE or environment if needed. The Visual Studio project uses:

```xml
  <AdditionalIncludeDirectories>.\lib-src\bass24\c;.\lib-src\bassasio12\c;%(AdditionalIncludeDirectories)</AdditionalIncludeDirectories>
```

## 3. Build the Executable

Compile the project with a single command from the repository root.

1. Open a terminal in the project folder.
2. Run:

```bash
   make
```

1. The top-level **Makefile** pulls in `..\makefile.in`, sets the target and flags:

```makefile
   include ..\makefile.in
   TARGET = spispectrumplay.exe
   FLAGS += -mwindows
   LIBS = -lcomdlg32 -lgdi32 -lwinmm
```

1. On success, `spispectrumplay.exe` will appear in `Release\` or `Debug\` based on your configuration.

| Variable | Description |
| --- | --- |
| `TARGET` | Name of the generated executable |
| `FLAGS` | Compiler flags (e.g. `-mwindows`) |
| `LIBS` | Linker libraries for GUI & timers |


## 4. Prepare Runtime Files 📂

Deploy the executable and its dependencies for use.

- Copy the following into the same folder:
- **spispectrumplay.exe**
- **BASS.DLL**
- **BASSASIO.DLL**
- (Optional) Add a default WAV file named `testwav.wav` to launch without parameters.

| File | Purpose |
| --- | --- |
| spispectrumplay.exe | Main application binary |
| BASS.DLL | Core audio engine |
| BASSASIO.DLL | ASIO extension for low-latency output |
| testwav.wav | Default test audio file |


```card
{
    "title": "DLL Version Mismatch",
    "content": "Ensure BASS.DLL matches the BASSVERSION constant to avoid startup errors."
}
```

✨ You’re now ready to launch **spispectrumplay**. Simply double-click the EXE or run it from a console!