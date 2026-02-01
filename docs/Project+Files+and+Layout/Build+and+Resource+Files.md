# Project Files and Layout - Build and Resource Files

This section details the core build and resource files that define how **spispectrumplay** is compiled, linked, and how its basic GUI resources are configured. Understanding these files will help you customize the build process and extend the application’s UI assets.

---

## Makefile

The `makefile` provides a minimal NMAKE-based build setup for creating the Windows GUI executable. It delegates most settings to a common include and then specifies the target, flags, and libraries.

```make
include ..\makefile.in
TARGET = spispectrumplay.exe
FLAGS += -mwindows
LIBS = -lcomdlg32 -lgdi32 -lwinmm

all: $(TARGET)

clean:
	$(RM) $(OUTDIR)\$(TARGET)
```

- **include ..\makefile.in**

Imports shared compiler/linker settings, including paths to BASS/BASSASIO headers and libraries.

- **TARGET = spispectrumplay.exe**

Defines the output executable name.

- **FLAGS += -mwindows**

Adds the `-mwindows` switch to produce a GUI app (no console window).

- **LIBS = -lcomdlg32 -lgdi32 -lwinmm**
- `comdlg32` for common dialogs (Open File dialog).
- `gdi32` for drawing the spectrum.
- `winmm` for multimedia timers used to drive spectrum updates.
- **all** and **clean**
- `all` is the default build target.
- `clean` removes the generated executable.

---

## resource.h

The Visual C++-generated `resource.h` defines numeric identifiers for GUI resources. Currently it only includes the application icon ID and placeholders for future commands or controls.

```c
//{{NO_DEPENDENCIES}}
// Microsoft Visual C++ generated include file.
// Used by spispectrumplay_asio.rc

#define IDI_ICON1                       101

// Next default values for new objects
#ifndef APSTUDIO_READONLY_SYMBOLS
#define _APS_NEXT_RESOURCE_VALUE        102
#define _APS_NEXT_COMMAND_VALUE         40001
#define _APS_NEXT_CONTROL_VALUE         1001
#define _APS_NEXT_SYMED_VALUE           101
#endif
```

- **IDI_ICON1 = 101**

Identifier for `icon1.ico`, displayed in the window’s title bar and taskbar.

- **\***

Auto-generated placeholders to ensure new resources get unique IDs.

This header is referenced by the resource script `spispectrumplay_asio.rc`, which embeds the icon and other resources.

---

## debug.txt (Example Log)

An example `debug.txt` illustrates how the app logs device detection at startup. Reviewing this can help understand and troubleshoot audio-device selection.

```text
using default non-asio audio device
Headphones (TP 18 Stereo) maps to 1
```

- **“using default non-asio audio device”**

Indicates no ASIO device was specified or found, so the app fell back to the default system audio device.

- **Device mapping line**

Shows the friendly name and its internal device ID.

Such logs are produced when `pFILE` (a `FILE*`) is open, enabling developers to verify correct device enumeration and selection logic.

---

```card
{
    "title": "Pro Tip",
    "content": "Modify the common `makefile.in` to adjust include paths or BASS/BASSASIO versions across multiple projects."
}
```

```card
{
    "title": "Debug Tip",
    "content": "Inspect `debug.txt` to quickly confirm whether ASIO or non-ASIO devices were initialized."
}
```

These files form the foundation of the build and basic resource layout for **spispectrumplay**, ensuring a streamlined GUI build and providing hooks for future resource additions.