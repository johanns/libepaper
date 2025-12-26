#!/usr/bin/env python3
"""
Generate test images for e-Paper bitmap drawing examples.
Requires: pip install pillow
"""

from PIL import Image, ImageDraw, ImageFont
import os
import math

# Create images directory
os.makedirs('images', exist_ok=True)

def create_checkerboard(size=64, square_size=8):
    """Create a checkerboard pattern."""
    img = Image.new('L', (size, size), 255)
    draw = ImageDraw.Draw(img)

    for y in range(0, size, square_size):
        for x in range(0, size, square_size):
            if ((x // square_size) + (y // square_size)) % 2 == 0:
                draw.rectangle([x, y, x + square_size - 1, y + square_size - 1], fill=0)

    return img

def create_gradient(width=128, height=64, direction='horizontal'):
    """Create a gradient image."""
    img = Image.new('L', (width, height))
    pixels = img.load()

    for y in range(height):
        for x in range(width):
            if direction == 'horizontal':
                value = int((x / width) * 255)
            elif direction == 'vertical':
                value = int((y / height) * 255)
            else:  # diagonal
                value = int(((x + y) / (width + height)) * 255)
            pixels[x, y] = value

    return img

def create_circle_pattern(size=100):
    """Create concentric circles."""
    img = Image.new('L', (size, size), 255)
    draw = ImageDraw.Draw(img)

    center = size // 2
    for r in range(5, center, 10):
        draw.ellipse([center - r, center - r, center + r, center + r],
                     outline=0, width=3)

    return img

def create_test_logo(size=80):
    """Create a simple test logo."""
    img = Image.new('L', (size, size), 255)
    draw = ImageDraw.Draw(img)

    # Draw a simple "EP" logo for E-Paper
    margin = 10

    # E
    draw.rectangle([margin, margin, margin + 15, margin + 60], fill=0)
    draw.rectangle([margin, margin, margin + 25, margin + 10], fill=0)
    draw.rectangle([margin, margin + 25, margin + 20, margin + 35], fill=0)
    draw.rectangle([margin, margin + 50, margin + 25, margin + 60], fill=0)

    # P
    x_offset = margin + 35
    draw.rectangle([x_offset, margin, x_offset + 10, margin + 60], fill=0)
    draw.ellipse([x_offset + 10, margin, x_offset + 35, margin + 30], outline=0, width=3)

    return img

def create_icon_set():
    """Create a set of small icons."""
    icons = {}

    # Battery icon
    battery = Image.new('L', (32, 16), 255)
    draw = ImageDraw.Draw(battery)
    draw.rectangle([0, 2, 28, 13], outline=0, width=2)
    draw.rectangle([28, 5, 31, 10], fill=0)
    draw.rectangle([3, 5, 12, 10], fill=0)  # Fill level
    icons['battery'] = battery

    # WiFi icon
    wifi = Image.new('L', (24, 24), 255)
    draw = ImageDraw.Draw(wifi)
    center_x, center_y = 12, 20
    for r in [4, 8, 12]:
        draw.arc([center_x - r, center_y - r, center_x + r, center_y + r],
                 start=225, end=315, fill=0, width=2)
    draw.ellipse([center_x - 2, center_y - 2, center_x + 2, center_y + 2], fill=0)
    icons['wifi'] = wifi

    # Clock icon
    clock = Image.new('L', (32, 32), 255)
    draw = ImageDraw.Draw(clock)
    draw.ellipse([2, 2, 30, 30], outline=0, width=2)
    center = 16
    draw.line([center, center, center, center - 8], fill=0, width=2)  # Hour
    draw.line([center, center, center + 6, center], fill=0, width=2)  # Minute
    icons['clock'] = clock

    # Warning icon
    warning = Image.new('L', (32, 32), 255)
    draw = ImageDraw.Draw(warning)
    points = [(16, 2), (30, 30), (2, 30)]
    draw.polygon(points, outline=0, width=2)
    draw.line([16, 10, 16, 20], fill=0, width=2)
    draw.ellipse([14, 23, 18, 27], fill=0)
    icons['warning'] = warning

    return icons

def create_photo_like(size=150):
    """Create a photo-like test image with various tones."""
    img = Image.new('L', (size, size), 128)
    draw = ImageDraw.Draw(img)

    # Create a scene with sky, ground, and a simple house
    # Sky (light)
    draw.rectangle([0, 0, size, size // 2], fill=200)

    # Ground (medium)
    draw.rectangle([0, size // 2, size, size], fill=100)

    # House (dark rectangle)
    house_x = size // 3
    house_y = size // 2 - 30
    house_w = size // 3
    house_h = 40
    draw.rectangle([house_x, house_y, house_x + house_w, house_y + house_h], fill=50)

    # Roof (triangle)
    roof_points = [
        (house_x - 10, house_y),
        (house_x + house_w // 2, house_y - 20),
        (house_x + house_w + 10, house_y)
    ]
    draw.polygon(roof_points, fill=30)

    # Windows
    win_size = 10
    draw.rectangle([house_x + 10, house_y + 10,
                   house_x + 10 + win_size, house_y + 10 + win_size], fill=150)
    draw.rectangle([house_x + house_w - 20, house_y + 10,
                   house_x + house_w - 10, house_y + 10 + win_size], fill=150)

    return img

def create_text_image(text="E-Paper", size=(120, 40)):
    """Create an image with text."""
    img = Image.new('L', size, 255)
    draw = ImageDraw.Draw(img)

    # Try to use a default font, fall back to basic if not available
    try:
        font = ImageFont.truetype("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf", 24)
    except:
        font = ImageFont.load_default()

    # Center the text
    bbox = draw.textbbox((0, 0), text, font=font)
    text_width = bbox[2] - bbox[0]
    text_height = bbox[3] - bbox[1]
    x = (size[0] - text_width) // 2
    y = (size[1] - text_height) // 2

    draw.text((x, y), text, fill=0, font=font)

    return img

def create_qr_code_like(size=64):
    """Create a QR-code-like pattern."""
    img = Image.new('L', (size, size), 255)
    draw = ImageDraw.Draw(img)

    # Simple pseudo-random pattern
    import random
    random.seed(42)  # Fixed seed for reproducibility

    cell_size = 4
    for y in range(0, size, cell_size):
        for x in range(0, size, cell_size):
            if random.random() > 0.5:
                draw.rectangle([x, y, x + cell_size - 1, y + cell_size - 1], fill=0)

    # Add corner markers
    marker_size = 12
    for corner in [(0, 0), (size - marker_size, 0), (0, size - marker_size)]:
        draw.rectangle([corner[0], corner[1],
                       corner[0] + marker_size, corner[1] + marker_size],
                      outline=0, width=2)

    return img

def main():
    print("Generating test images...")

    # Checkerboard patterns
    print("  Creating checkerboard patterns...")
    create_checkerboard(64, 8).save('images/checkerboard_64.png')
    create_checkerboard(64, 8).save('images/checkerboard_64.bmp')
    create_checkerboard(32, 4).save('images/checkerboard_32.png')
    create_checkerboard(128, 16).save('images/checkerboard_128.jpg', quality=95)

    # Gradients
    print("  Creating gradients...")
    create_gradient(128, 64, 'horizontal').save('images/gradient_horizontal.png')
    create_gradient(64, 128, 'vertical').save('images/gradient_vertical.png')
    create_gradient(100, 100, 'diagonal').save('images/gradient_diagonal.bmp')

    # Patterns
    print("  Creating patterns...")
    create_circle_pattern(100).save('images/circles.png')
    create_circle_pattern(100).save('images/circles.jpg', quality=90)

    # Logo
    print("  Creating logo...")
    create_test_logo(80).save('images/logo.png')
    create_test_logo(80).save('images/logo.bmp')
    create_test_logo(120).save('images/logo_large.png')

    # Icons
    print("  Creating icons...")
    icons = create_icon_set()
    for name, icon in icons.items():
        icon.save(f'images/icon_{name}.png')
        icon.save(f'images/icon_{name}.bmp')

    # Photo-like image
    print("  Creating photo-like images...")
    create_photo_like(150).save('images/photo_test.png')
    create_photo_like(150).save('images/photo_test.jpg', quality=85)
    create_photo_like(200).save('images/photo_test_large.png')

    # Text images
    print("  Creating text images...")
    create_text_image("E-Paper", (120, 40)).save('images/text_epaper.png')
    create_text_image("Hello!", (100, 30)).save('images/text_hello.png')
    create_text_image("TEST", (80, 30)).save('images/text_test.bmp')

    # QR-like pattern
    print("  Creating QR-like pattern...")
    create_qr_code_like(64).save('images/qr_like.png')
    create_qr_code_like(64).save('images/qr_like.bmp')

    # Various sizes for scaling tests
    print("  Creating images for scaling tests...")
    create_test_logo(40).save('images/logo_small.png')
    create_test_logo(160).save('images/logo_xlarge.png')

    print(f"\n✓ Generated {len(os.listdir('images'))} test images in images/")
    print("\nGenerated files:")
    for filename in sorted(os.listdir('images')):
        filepath = os.path.join('images', filename)
        size_kb = os.path.getsize(filepath) / 1024
        print(f"  - {filename:30s} ({size_kb:6.2f} KB)")

if __name__ == '__main__':
    main()

