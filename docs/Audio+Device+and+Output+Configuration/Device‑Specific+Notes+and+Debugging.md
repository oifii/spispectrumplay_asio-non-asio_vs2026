# Audio Device and Output Configuration – Device-Specific Notes and Debugging

This section explains how the application selects and configures audio output devices, with a focus on non-ASIO device handling, dynamic default detection, logging for debugging, and fallback mechanisms. Understanding these details helps troubleshoot audio-device issues and customize device selection.

## Default Non-ASIO Device Name ⚙️

The global variable `**global_audiodevicename_default**` holds a sane default for non-ASIO playback devices. It is initialized at startup but is overwritten if the system reports a different default.

```cpp
// Initialized at startup… but updated when enumerating devices
string global_audiodevicename_default = "Speakers (USB AUDIO  CODEC)";
int    global_deviceid               = 0;
FILE*  pFILE                         = nullptr;
map<string,int> global_nonasiodevicemap;
```

- **Purpose**: Provide a fallback device name before enumeration
- **Usage**: Only used for non-ASIO device matching; ASIO uses explicit names
- **Reference**:

## Device Enumeration and Dynamic Update

When no ASIO device is selected (or the requested ASIO device isn’t found), the app enumerates non-ASIO devices via `**BASS_GetDeviceInfo**`. It builds a name→ID map, detects which device is marked default by the OS, and updates `global_audiodevicename_default` and `global_deviceid` accordingly.

```cpp
fprintf(pFILE, "list of installed non-asio audio devices\n");
fflush(pFILE);

BASS_DEVICEINFO di;
for (int i = 0; BASS_GetDeviceInfo(i, &di); i++) {
    if (di.flags & BASS_DEVICE_ENABLED) {
        // build map
        global_nonasiodevicemap[di.name] = i;

        // detect OS default
        if (di.flags & BASS_DEVICE_DEFAULT) {
            global_audiodevicename_default = di.name;
            global_deviceid               = i;
            fprintf(pFILE,
                    "%s maps to %d (default non-asio audio device)\n",
                    di.name.c_str(), i);
            fflush(pFILE);
        } else {
            fprintf(pFILE, "%s maps to %d\n", di.name.c_str(), i);
            fflush(pFILE);
        }
    }
}
```

- `**BASS_DEVICE_DEFAULT**` flag indicates the system’s chosen default device.
- `**global_nonasiodevicemap**` stores each device’s name and ID for lookups.
- **Reference**:

## Logging 🛠️

All key steps in device enumeration and selection are logged to `**pFILE**` (if opened). This aids diagnosing mismatches between requested and actual audio devices.

| Log Message | Meaning | Reference |
| --- | --- | --- |
| `list of installed non-asio audio devices` | Start of non-ASIO enumeration |  |
| `<Name> maps to <ID> (default non-asio audio device)` | OS default device detected and its ID |  |
| `<Name> maps to <ID>` | Other enabled non-ASIO device detected |  |
| `using user-specified non-asio audio device` | User passed a device name via command-line and it was found |  |
| `using default non-asio audio device` | No user device; falling back to OS default |  |
| `using first non-asio audio device id = <ID>` | No default or user device; using first real device (ID 1) |  |


### Example: User-Specified vs Default

```cpp
auto it = global_nonasiodevicemap.find(global_audiodevicename);
if (it != global_nonasiodevicemap.end()) {
    global_deviceid = it->second;
    fprintf(pFILE, "using user-specified non-asio audio device\n");
    fprintf(pFILE, "%s maps to %d\n",
            global_audiodevicename.c_str(), global_deviceid);
} else {
    // fall back to OS default…
    it = global_nonasiodevicemap.find(global_audiodevicename_default);
    if (it != global_nonasiodevicemap.end()) {
        global_deviceid = it->second;
        fprintf(pFILE, "using default non-asio audio device\n");
        fprintf(pFILE, "%s maps to %d\n",
                global_audiodevicename_default.c_str(), global_deviceid);
    }
    // …or fallback further below
}
```

## No-Sound Fallback

If neither a user-specified nor an OS default device is available, the app assumes **device ID 1** corresponds to the first real audio output. This avoids silently selecting **ID 0**, which is a “no sound” device on many systems.

```cpp
// Fallback when no device found:
global_deviceid = 1; 
fprintf(pFILE,
        "using first non-asio audio device id = %d\n",
        global_deviceid);
```

- **Rationale**:
- **ID 0** = “no sound” device
- **ID 1** = first real output device (MME/DS on Windows)
- **Reference**:

| Device ID | Description |
| --- | --- |
| 0 | No sound (dummy) |
| 1 | First real output |


---

These notes ensure predictable audio-device selection, transparent debugging through logs, and robust fallback behavior when devices aren’t explicitly specified or available.