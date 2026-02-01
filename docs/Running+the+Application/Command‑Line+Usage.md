# Running the Application – Command-Line Usage

The **spispectrumplay** application accepts a series of **positional** and **optional** command-line arguments. It relies on the Windows API function `CommandLineToArgvA` to split the raw command line into an array of C-strings, then maps each index to a specific configuration parameter. Trailing arguments that aren’t provided simply retain their **default** values.

## Argument Parsing Process

At startup, the code in `spispectrumplay.cpp` performs the following steps:

```cpp
LPSTR* szArgList;
int argCount;
szArgList = CommandLineToArgvA(GetCommandLine(), &argCount);
if (!szArgList) {
    MessageBox(NULL, "Unable to parse command line", "Error", MB_OK);
    return 10;
}
// Parse szArgList[1] … szArgList[10] into globals
LocalFree(szArgList);
```

This ensures robust handling of **quoted** strings and whitespace.

## 🎚 Command-Line Parameters

All parameters are **optional** and **positional**. Omit any trailing values to apply the defaults listed below.

| Index | Argument Name | Description | Default | Notes |
| --- | --- | --- | --- | --- |
| :-----: | :------------------------ | :-------------------------------------------------------------------------------------------------- | :-------------: | :----------------------------------------------------------------------------------------------------------------------------------------- |
| (1) | `filename` | Path to the audio file to play | `testwav.wav` | Supports any format BASS handles (WAV, MP3, OGG, …). |
| (2) | `playSeconds` | Playback duration in seconds | `-1.0` | Negative → play once without forced stop; positive → sets a timer to post `WM_DESTROY` when expired. |
| (3) | `posX` | Initial X-position of the visualization window | `200` | Integer pixels from the left edge of the primary display. |
| (4) | `posY` | Initial Y-position of the visualization window | `200` | Integer pixels from the top edge of the primary display. |
| (5) | `specmode` | Initial visualization mode | `0` | Integer 0–3; see **Visualization Modes** for presets. |
| (6) | `audiodeviceName` | Desired audio device name (ASIO or non-ASIO) | *(empty)* | If matching an ASIO driver, uses that; otherwise scans non-ASIO devices for a match or defaults to system default. |
| (7) | `asioLeftChannelIndex` | ASIO output channel index for the left channel | `6` | Even-numbered indices (0,2,4,6,8…) select which ASIO output port carries the left feed. |
| (8) | `asioRightChannelIndex` | ASIO output channel index for the right channel | `7` | Odd-numbered indices (1,3,5,7,9…) select which ASIO output port carries the right feed. |
| (9) | `alpha` | Window transparency (0–255) | `200` | Applied via `SetLayeredWindowAttributes`; lower = more transparent. |
| (10) | `volume` | Initial playback volume (float) | `1.0` | Range 0.0–1.0; clamped and applied with `BASS_ChannelSetAttribute(chan, BASS_ATTRIB_VOL, global_fvolume)`. |


## Example Invocation

```bash
spispectrumplay.exe "myfile.wav" 30 200 200 1 "E-MU ASIO" 0 1 220 0.8
```

- **File**: `myfile.wav`
- **Duration**: 30 seconds
- **Window position**: (200, 200)
- **Visualization mode**: 1
- **Audio device**: `"E-MU ASIO"`
- **ASIO channels**: left = 0, right = 1
- **Window alpha**: 220
- **Volume**: 80%

## Key Notes

- **Quoting**: Wrap any argument containing spaces in **double quotes** to ensure correct parsing.
- **Defaults**: Omitting all arguments plays `testwav.wav` to completion with system defaults.
- **Error Handling**: If command-line parsing fails, an error box appears and the app exits with code 10.

```card
{
    "title": "Quoting Arguments",
    "content": "Wrap parameters with spaces in double quotes to ensure they are parsed correctly."
}
```

This configuration approach lets users fully tailor playback, window placement, visualization style, and audio routing directly from the command line.