# Test Images for E-Paper Bitmap Drawing

This directory contains test images in various formats (PNG, BMP, JPEG) for testing the bitmap drawing functionality.

## How to Generate

These images were generated using the Python script in the parent directory:

```bash
cd examples
python3 generate_images.py
```

## Image Categories

### Checkerboard Patterns
- `checkerboard_32.png` - 32×32 small checkerboard
- `checkerboard_64.png` - 64×64 medium checkerboard (PNG)
- `checkerboard_64.bmp` - 64×64 medium checkerboard (BMP)
- `checkerboard_128.jpg` - 128×128 large checkerboard (JPEG)

**Use for:** Testing pattern rendering, scaling accuracy, format support

### Gradients
- `gradient_horizontal.png` - 128×64 horizontal gradient
- `gradient_vertical.png` - 64×128 vertical gradient
- `gradient_diagonal.bmp` - 100×100 diagonal gradient

**Use for:** Testing grayscale conversion, color mapping to display modes

### Patterns
- `circles.png` - 100×100 concentric circles (PNG)
- `circles.jpg` - 100×100 concentric circles (JPEG)
- `qr_like.png` - 64×64 QR-code-like pattern (PNG)
- `qr_like.bmp` - 64×64 QR-code-like pattern (BMP)

**Use for:** Testing complex patterns, line rendering

### Logo Images
- `logo_small.png` - 40×40 small EP logo
- `logo.png` - 80×80 standard EP logo (PNG)
- `logo.bmp` - 80×80 standard EP logo (BMP)
- `logo_large.png` - 120×120 large EP logo
- `logo_xlarge.png` - 160×160 extra large EP logo

**Use for:** Testing different sizes, scaling up/down, format compatibility

### Icons
- `icon_battery.png` / `.bmp` - 32×16 battery indicator
- `icon_wifi.png` / `.bmp` - 24×24 WiFi symbol
- `icon_clock.png` / `.bmp` - 32×32 clock
- `icon_warning.png` / `.bmp` - 32×32 warning triangle

**Use for:** Testing small UI elements, icon rendering, practical use cases

### Photo-like Images
- `photo_test.png` - 150×150 simple scene (PNG)
- `photo_test.jpg` - 150×150 simple scene (JPEG)
- `photo_test_large.png` - 200×200 large scene

**Use for:** Testing complex grayscale images, photographic content

### Text Images
- `text_epaper.png` - 120×40 "E-Paper" text
- `text_hello.png` - 100×30 "Hello!" text
- `text_test.bmp` - 80×30 "TEST" text

**Use for:** Testing text rendering as bitmap, label generation

## Testing Scenarios

### Format Support Testing
```bash
# PNG support
images/logo.png
images/checkerboard_64.png

# BMP support
images/logo.bmp
images/checkerboard_64.bmp

# JPEG support
images/circles.jpg
images/photo_test.jpg
```

### Scaling Testing
```bash
# Scale up (small to large)
images/logo_small.png (40×40) → display at 80×80

# Scale down (large to small)
images/logo_xlarge.png (160×160) → display at 80×80

# Aspect ratio testing
images/icon_battery.png (32×16) → display at 64×32
```

### Grayscale Conversion Testing
```bash
# Gradients
images/gradient_horizontal.png
images/gradient_vertical.png

# Photo-like
images/photo_test.png
```

### Practical Use Cases
```bash
# UI Icons
images/icon_battery.png
images/icon_wifi.png
images/icon_clock.png

# Logos/Branding
images/logo.png

# Patterns/Decorations
images/circles.png
images/qr_like.png
```

## File Sizes

| Format | Small (~KB) | Medium (~KB) | Large (~KB) |
|--------|-------------|--------------|-------------|
| PNG    | 0.1-0.3     | 0.3-0.5      | 0.5-1.0     |
| BMP    | 1.5-5.0     | 5.0-10.0     | 10.0-20.0   |
| JPEG   | 0.7-2.0     | 2.0-6.0      | 6.0-15.0    |

**Note:** PNG is most efficient for these test images due to their simple patterns. BMP is uncompressed and largest. JPEG is good for photo-like content.

## Usage in Code

### Example 1: Load and Display Logo
```cpp
draw.draw_bitmap_from_file(10, 10, "images/logo.png");
```

### Example 2: Load Icon with Scaling
```cpp
draw.draw_bitmap_from_file(100, 10, "images/icon_battery.png", 48, 24);
```

### Example 3: Test Multiple Formats
```cpp
// PNG
draw.draw_bitmap_from_file(10, 10, "images/circles.png");

// BMP
draw.draw_bitmap_from_file(10, 70, "images/circles.bmp");

// JPEG
draw.draw_bitmap_from_file(10, 130, "images/circles.jpg");
```

### Example 4: Scaling Comparison
```cpp
// Original size
draw.draw_bitmap_from_file(10, 10, "images/logo.png");

// 2x scaled
draw.draw_bitmap_from_file(100, 10, "images/logo.png", 160, 160);

// 0.5x scaled
draw.draw_bitmap_from_file(10, 100, "images/logo.png", 40, 40);
```

## Regenerating Images

If you need to regenerate the test images (e.g., after modifying the script):

```bash
# Remove old images
rm -rf images

# Run generation script
python3 generate_images.py
```

## Adding Custom Test Images

To add your own test images:

1. Copy images to this directory
2. Use supported formats: PNG, JPEG, BMP, GIF, TGA, etc.
3. Recommended sizes: 16×16 to 200×200 pixels
4. Use grayscale or color (will be auto-converted)
5. Test with the bitmap_example program

## License

These test images were generated programmatically and are in the public domain.
You may use them freely for testing, development, and any other purpose.

