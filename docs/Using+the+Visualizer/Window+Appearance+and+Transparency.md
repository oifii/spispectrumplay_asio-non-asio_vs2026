# Using the Visualizer – Window Appearance and Transparency

This section describes how the spectrum visualizer window is styled, how its semi-transparent layered surface is created, and how rendering is performed using an 8-bit palettized DIB. These techniques allow the visualizer to blend smoothly with any desktop background while updating in real time.

## Window Styles and Alpha Blending

The visualizer window is made **layered** to support per-window alpha transparency:

- **Extended style**: `WS_EX_LAYERED` is added to the window via `SetWindowLong`.
- **Alpha value**: the global variable `global_alpha` (0–255) controls opacity.
- **API calls**:

```cpp
  // in WM_CREATE handler
  SetWindowLong(hWnd, GWL_EXSTYLE,
      GetWindowLong(hWnd, GWL_EXSTYLE) | WS_EX_LAYERED);
  SetLayeredWindowAttributes(hWnd, 0, global_alpha, LWA_ALPHA);
```

## Visualization Surface as an 8-bit DIB

The spectrum is drawn into a fixed-size, 8-bit Device Independent Bitmap (DIB). This allows very fast pixel updates by writing directly into memory.

### Dimensions and Format

| Property | Macro / Value |
| --- | --- |
| Width | `SPECWIDTH` = 400 |
| Height | `SPECHEIGHT` = 127 |
| Pixel depth | 8 bits (palettized) |
| Palette | 256 entries (0–255 indices) |


These dimensions are defined in code:

```cpp
#define SPECWIDTH  400   // display width  
#define SPECHEIGHT 127   // display height  
```

### Bitmap Creation

A DIB section and off-screen memory DC are created once in `WM_CREATE`:

```cpp
BITMAPINFOHEADER bi = {0};
bi.biSize        = sizeof(bi);
bi.biWidth       = SPECWIDTH;
bi.biHeight      = SPECHEIGHT;   // bottom-up orientation
bi.biPlanes      = 1;
bi.biBitCount    = 8;
bi.biClrUsed     = bi.biClrImportant = 256;

// allocate the bitmap and get a pointer to its pixel buffer
specbmp = CreateDIBSection(
    NULL, (BITMAPINFO*)&bi,
    DIB_RGB_COLORS,
    (void**)&specbuf,
    NULL, 0
);

specdc = CreateCompatibleDC(NULL);
SelectObject(specdc, specbmp);
```

## Custom Palette Configuration 🎨

A **custom palette** maps each pixel index (0–255) to an RGB color. By default, indices 1–127 fade from red to green; higher indices add spectral highlights.

```cpp
// red→green gradient for intensity 1–127
for (int i = 1; i < 128; i++) {
    pal[i].rgbRed   = 256 - 2 * i;
    pal[i].rgbGreen = 2 * i;
}

// additional bands for highlighting
for (int i = 0; i < 32; i++) {
    pal[128 + i       ].rgbBlue  = 8 * i;
    pal[128 + 32 + i  ].rgbBlue  = 255;
    pal[128 + 32 + i  ].rgbGreen = 8 * i;
    // …more ranges…
}
```

Alternate schemes (grayscale, solid color, blue→green, etc.) are provided as commented blocks and can be enabled by editing this palette initialization .

## Rendering Pipeline

### Spectrum Update Callback

The timer-driven `UpdateSpectrum` function fills `specbuf` based on FFT or waveform data. Once drawing is complete, it **blits** the off-screen bitmap to the window:

```cpp
void CALLBACK UpdateSpectrum(...){
    // …fill specbuf[x]…
    HDC dc = GetDC(win);
    BitBlt(dc,
           0, 0, SPECWIDTH, SPECHEIGHT,
           specdc,
           0, 0,
           SRCCOPY
    );
    ReleaseDC(win, dc);
}
```

### WM_PAINT Handling

On any invalidate, the window procedure simply copies the existing bitmap to the screen:

```cpp
case WM_PAINT:
    if (GetUpdateRect(hWnd, NULL, FALSE)) {
        PAINTSTRUCT ps;
        HDC dc = BeginPaint(hWnd, &ps);
        BitBlt(dc,
               0, 0, SPECWIDTH, SPECHEIGHT,
               specdc,
               0, 0,
               SRCCOPY
        );
        EndPaint(hWnd, &ps);
    }
    return 0;
```

## Integration and Usage

- **Opacity control**: adjust `global_alpha` at startup or via UI to make the visualizer more or less transparent.
- **Mouse interactions**: clicking toggles modes; shift-click near the bottom seeks playback while maintaining transparency.
- **Device independence**: rendering code is agnostic to ASIO vs. non-ASIO audio paths.
- **Performance**: using an 8-bit palettized DIB and a memory DC ensures minimal CPU/GDI overhead at 40 Hz updates.

By combining **layered window styling** with an **8-bit DIB back-end**, the visualizer achieves smooth, real-time spectrum rendering that floats seamlessly over any desktop backdrop.