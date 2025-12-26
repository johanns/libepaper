# Test Images Visual Guide

This guide provides a visual reference for all test images included in the `images/` directory.

## Quick Start

Generate test images:
```bash
python3 generate_test_images.py
```

Run the bitmap example:
```bash
cd build
sudo ./examples/bitmap_example
```

## Test Image Gallery

### 📋 Checkerboard Patterns

| Image | Size | Format | Description | Use Case |
|-------|------|--------|-------------|----------|
| `checkerboard_32.png` | 32×32 | PNG | Small 4×4 checkerboard | Testing small patterns |
| `checkerboard_64.png` | 64×64 | PNG | Medium 8×8 checkerboard | Standard pattern test |
| `checkerboard_64.bmp` | 64×64 | BMP | Same pattern, BMP format | Format comparison |
| `checkerboard_128.jpg` | 128×128 | JPEG | Large 16×16 checkerboard | Large pattern, JPEG test |

**Visual Pattern:**
```
█░█░█░█░
░█░█░█░█
█░█░█░█░
░█░█░█░█
```

### 🎨 Gradients

| Image | Size | Format | Direction | Use Case |
|-------|------|--------|-----------|----------|
| `gradient_horizontal.png` | 128×64 | PNG | → Horizontal | Grayscale mapping test |
| `gradient_vertical.png` | 64×128 | PNG | ↓ Vertical | Vertical gradient |
| `gradient_diagonal.bmp` | 100×100 | BMP | ↘ Diagonal | Diagonal blend |

**Visual Pattern:**
```
Horizontal: ░░░░▒▒▒▒████
Vertical:   ░
            ░
            ▒
            ▒
            █
            █
```

### 🔵 Pattern Circles

| Image | Size | Format | Description | Use Case |
|-------|------|--------|-------------|----------|
| `circles.png` | 100×100 | PNG | Concentric circles | Line rendering, patterns |
| `circles.jpg` | 100×100 | JPEG | Same, JPEG format | Format comparison |

**Visual Pattern:**
```
    ████
  ██░░░░██
 █░░████░░█
█░░█░░░░█░░█
 █░░████░░█
  ██░░░░██
    ████
```

### 🏢 Logo Images (EP Logo)

| Image | Size | Format | Description | Use Case |
|-------|------|--------|-------------|----------|
| `logo_small.png` | 40×40 | PNG | Tiny logo | Icon size |
| `logo.png` | 80×80 | PNG | Standard logo | Default size |
| `logo.bmp` | 80×80 | BMP | Standard logo, BMP | Format test |
| `logo_large.png` | 120×120 | PNG | Large logo | Upscaling test |
| `logo_xlarge.png` | 160×160 | PNG | Extra large logo | Downscaling test |

**Visual Pattern:**
```
███  ███
█    █ █
███  ███
█    █
███  █
```
*(Simplified "EP" logo)*

### 🔋 UI Icons

#### Battery Icon (32×16)
```
┌──────────────┬─┐
│████░░░░░░░░░░│█│
│████░░░░░░░░░░│█│
└──────────────┴─┘
```
- Files: `icon_battery.png`, `icon_battery.bmp`
- Use: Status indicators, UI elements

#### WiFi Icon (24×24)
```
    ╱╲╱╲
   ╱  ╲  ╲
  ╱    ╲   ╲
  ░░  ░░  ░░
    ░░  ░░
      ██
```
- Files: `icon_wifi.png`, `icon_wifi.bmp`
- Use: Connectivity indicators

#### Clock Icon (32×32)
```
   ┌───────┐
  ╱         ╲
 │     │     │
 │     └──   │
 │           │
  ╲         ╱
   └───────┘
```
- Files: `icon_clock.png`, `icon_clock.bmp`
- Use: Time display, scheduling

#### Warning Icon (32×32)
```
      ▲
     ╱ ╲
    ╱ │ ╲
   ╱  │  ╲
  ╱   ●   ╲
 ╱─────────╲
```
- Files: `icon_warning.png`, `icon_warning.bmp`
- Use: Alerts, error states

### 🏠 Photo-like Images

| Image | Size | Format | Description |
|-------|------|--------|-------------|
| `photo_test.png` | 150×150 | PNG | Simple scene with house |
| `photo_test.jpg` | 150×150 | JPEG | Same scene, JPEG |
| `photo_test_large.png` | 200×200 | PNG | Larger version |

**Scene Description:**
```
╔═══════════╗  <- Light sky
║  ┌─┐       ║
║ ╱   ╲      ║  <- Roof
║ │ □ □ │    ║  <- House with windows
╠═══════════╣
║░░░░░░░░░░░║  <- Ground
╚═══════════╝
```

### 📝 Text Images

| Image | Size | Format | Text Content |
|-------|------|--------|--------------|
| `text_epaper.png` | 120×40 | PNG | "E-Paper" |
| `text_hello.png` | 100×30 | PNG | "Hello!" |
| `text_test.bmp` | 80×30 | BMP | "TEST" |

**Use Cases:**
- Testing text-as-bitmap rendering
- Label generation
- Pre-rendered text for faster display

### 🔲 QR-like Pattern

| Image | Size | Format | Description |
|-------|------|--------|-------------|
| `qr_like.png` | 64×64 | PNG | Pseudo-random pattern |
| `qr_like.bmp` | 64×64 | BMP | Same pattern, BMP |

**Visual Pattern:**
```
┌──┐░█░░█░█┌──┐
│  │█░█░█░░│  │
│  │░░░█░░░│  │
└──┘█░░░█░░└──┘
█░░█░░█░░░░░░░█
░█░░█░░░█░█░░█░
┌──┐░█░░█░░░░░█
│  │░░█░█░░█░░░
│  │█░░░░░█░█░█
└──┘░█░█░░░░░█░
```

## Testing Scenarios

### 1. Format Compatibility Test

Test all three major formats with the same content:

```cpp
// PNG
draw.draw_bitmap_from_file(10, 10, "images/logo.png");

// BMP
draw.draw_bitmap_from_file(100, 10, "images/logo.bmp");

// JPEG
draw.draw_bitmap_from_file(10, 100, "images/circles.jpg");
```

### 2. Scaling Test Suite

Test different scaling ratios:

```cpp
// Original (80×80)
draw.draw_bitmap_from_file(10, 10, "images/logo.png");

// Scale down (40×40)
draw.draw_bitmap_from_file(100, 10, "images/logo.png", 40, 40);

// Scale up (160×160) - won't fit on 176×264 screen, but tests large scaling
draw.draw_bitmap_from_file(10, 100, "images/logo_small.png", 80, 80);
```

### 3. Grayscale Conversion Test

Test color/grayscale mapping:

```cpp
// Horizontal gradient (white → black)
draw.draw_bitmap_from_file(10, 10, "images/gradient_horizontal.png");

// Vertical gradient (white → black)
draw.draw_bitmap_from_file(10, 80, "images/gradient_vertical.png");

// Photo-like (multiple gray levels)
draw.draw_bitmap_from_file(10, 150, "images/photo_test.png", 80, 80);
```

### 4. UI Elements Test

Create a sample UI:

```cpp
// Status bar at top
draw.draw_bitmap_from_file(0, 0, "images/icon_wifi.png");
draw.draw_bitmap_from_file(30, 0, "images/icon_battery.png");
draw.draw_string(70, 5, "12:34", Font::font16(), Color::Black, Color::White);

// Main content
draw.draw_bitmap_from_file(10, 40, "images/logo.png");
draw.draw_string(100, 60, "E-Paper", Font::font20(), Color::Black, Color::White);

// Warning at bottom
draw.draw_bitmap_from_file(10, 200, "images/icon_warning.png");
draw.draw_string(50, 210, "Low Battery", Font::font12(), Color::Black, Color::White);
```

### 5. Pattern Fill Test

Use patterns as background fills:

```cpp
// Checkerboard background
for (int y = 0; y < 3; y++) {
    for (int x = 0; x < 3; x++) {
        draw.draw_bitmap_from_file(x * 64, y * 64, "images/checkerboard_64.png");
    }
}

// Overlay content
draw.draw_rectangle(50, 50, 125, 100, Color::White, DotPixel::Pixel1x1, DrawFill::Full);
draw.draw_string(55, 65, "Content", Font::font16(), Color::Black, Color::White);
```

## Image Size Reference

### Small (≤ 32px)
- Icons: battery, wifi
- Checkerboard small
- Logo small
- Best for: UI elements, status indicators

### Medium (33-100px)
- Logo standard
- Circles
- Gradients
- Checkerboard medium
- Best for: Main content, graphics

### Large (> 100px)
- Logo large/xlarge
- Photo test large
- Checkerboard large
- Best for: Full-screen content, backgrounds

## Display Mode Compatibility

### Black & White Mode
All images work but use only 2 colors:
- Threshold at gray level 128
- Good for: Icons, text, patterns
- Use: `logo.png`, `checkerboard_*.png`, `icon_*.png`

### 4-Level Grayscale Mode
Images use 4 gray levels:
- White, Gray1, Gray2, Black
- Good for: Photos, gradients, detailed images
- Use: `gradient_*.png`, `photo_test.png`, `circles.png`

## File Size Comparison

For a 64×64 image:
- **PNG**: ~0.13 KB (best compression for patterns)
- **BMP**: ~5.05 KB (uncompressed)
- **JPEG**: ~0.77 KB (good for photos, not ideal for patterns)

**Recommendation**: Use PNG for most e-paper applications.

## Performance Notes

### Loading Time (estimated for Raspberry Pi)
- Small images (< 1KB): < 10ms
- Medium images (1-5KB): 10-50ms
- Large images (> 5KB): 50-200ms

### Display Time
- Independent of format (all decoded to pixels)
- Depends only on final size: ~10ms per 10,000 pixels

### Memory Usage
- Temporary: `width × height × channels` bytes during loading
- Final: `width × height` Color enum values

## Troubleshooting

### Image Not Found
```
✗ Failed to load logo.png
```
**Solution**: Run from repository root or use absolute path

### Wrong Colors
All blacks appear white or vice versa?
**Solution**: Check display mode (BlackWhite vs Grayscale)

### Blocky Appearance
Scaled images look pixelated?
**Solution**: This is normal with nearest-neighbor scaling; prepare images at target size

### Slow Loading
Files take long to load?
**Solution**: Use PNG for best performance, pre-load frequently used images

## See Also

- **Test Images README**: `images/README.md`
- **Bitmap Drawing Guide**: `BITMAP_DRAWING.md`
- **Quick Reference**: `BITMAP_QUICK_REFERENCE.md`
- **Example Code**: `examples/bitmap_example.cpp`

