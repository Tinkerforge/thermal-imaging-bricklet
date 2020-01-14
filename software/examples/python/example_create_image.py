#!/usr/bin/env python
# -*- coding: utf-8 -*-

#
# Takes one thermal image and saves it as PNG
#

HOST = "localhost"
PORT = 4223
UID = "XYZ" # Change XYZ to the UID of your Thermal Imaging Bricklet

from tinkerforge.ip_connection import IPConnection
from tinkerforge.bricklet_thermal_imaging import BrickletThermalImaging

import time
import math
from PIL import Image

# Creates standard thermal image color palette (blue=cold, red=hot)
def get_thermal_image_color_palette():
    palette = []

    for x in range(256):
        x /= 255.0
        palette.append(int(round(255*math.sqrt(x))))                  # RED
        palette.append(int(round(255*pow(x, 3))))                     # GREEN
        if math.sin(2 * math.pi * x) >= 0:
            palette.append(int(round(255*math.sin(2 * math.pi * x)))) # BLUE
        else:
            palette.append(0)

    return palette

if __name__ == "__main__":
    ipcon = IPConnection() # Create IP connection
    ti = BrickletThermalImaging(UID, ipcon) # Create device object

    ipcon.connect(HOST, PORT) # Connect to brickd
    # Don't use device before ipcon is connected

    # Enable high contrast image transfer for getter
    ti.set_image_transfer_config(ti.IMAGE_TRANSFER_MANUAL_HIGH_CONTRAST_IMAGE)

    # If we change between transfer modes we have to wait until one more
    # image is taken after the mode is set and the first image is saved 
    # we can call get_high_contrast_image any time.
    time.sleep(0.5)

    # Get image data
    image_data = ti.get_high_contrast_image()

    # Make PNG with PIL
    image = Image.new('P', (80, 60))
    image.putdata(image_data)

    # This puts a color palette into place, if you 
    # remove this line you will get a greyscale image.
    image.putpalette(get_thermal_image_color_palette())

    # Scale to 800x600 and save thermal image!
    image.resize((80*10, 60*10), Image.ANTIALIAS).save('thermal_image.png')

    input("Press key to exit\n") # Use raw_input() in Python 2
    ipcon.disconnect()
