#!/usr/bin/env python
# -*- coding: utf-8 -*-

# Draws thermal image of Thermal Imaging Bricklet to LCD 128x64 Bricklet.
#
# You can find a video of this running on a HAT Brick here:
# https://www.tinkerforge.com/en/doc/Hardware/Bricks/HAT_Brick.html#description

HOST = "localhost"
PORT = 4223
UID_TI  = "ABC"  # Change ABC to the UID of your Thermal Imaging Bricklet
UID_LCD = "DEF"  # Change DEF to the UID of your LCD 128x64
LCD_WIDTH  = 128 # Columns LCD
LCD_HEIGHT = 64  # Rows LCD
TI_WIDTH   = 80  # Columns Thermal Image
TI_HEIGHT  = 60  # Rows Thermal Image
THRESHOLD  = 100 # Thermal threshold value

from tinkerforge.ip_connection import IPConnection
from tinkerforge.bricklet_thermal_imaging import BrickletThermalImaging
from tinkerforge.bricklet_lcd_128x64 import BrickletLCD128x64

# Callback function for high contrast image callback
def cb_high_contrast_image(lcd, image):
    # Convert 8 bit thermal image into 1 bit image for LCD 128x64
    image_bw = []
    for x in image:
        image_bw.append(x > THRESHOLD)

    # Draw 1 bit image to to center of LCD 128x64
    lcd.write_pixels(24, 2, TI_WIDTH+24-1, TI_HEIGHT+2-1, image_bw)

if __name__ == "__main__":
    ipcon = IPConnection() # Create IP connection

    # Create device objects
    ti = BrickletThermalImaging(UID_TI, ipcon)
    lcd = BrickletLCD128x64(UID_LCD, ipcon)

    ipcon.connect(HOST, PORT) # Connect to brickd
    # Don't use device before ipcon is connected

    # Clear image
    lcd.clear_display()

    # Register high contrast image callback to function cb_high_contrast_image
    ti.register_callback(ti.CALLBACK_HIGH_CONTRAST_IMAGE, lambda x: cb_high_contrast_image(lcd, x))

    # Enable high contrast image transfer for callback
    ti.set_image_transfer_config(ti.IMAGE_TRANSFER_CALLBACK_HIGH_CONTRAST_IMAGE)

    raw_input("Press key to exit\n") # Use input() in Python 3
    ipcon.disconnect()
