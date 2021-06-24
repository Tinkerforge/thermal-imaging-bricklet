#!/usr/bin/env python
# -*- coding: utf-8 -*-

HOST = "localhost"
PORT = 4223
UID = "XYZ" # Change XYZ to the UID of your Thermal Imaging Bricklet

import cv2
import numpy

from tinkerforge.ip_connection import IPConnection
from tinkerforge.bricklet_thermal_imaging import BrickletThermalImaging

# Callback function for temperature image callback
def cb_temperature_image(image):
    # image is an tuple of size 80*60 with a 16 bit grey value for each element
    reshaped_image = numpy.array(image, dtype=numpy.uint16).reshape(60, 80)

    # scale image 8x
    resized_image = cv2.resize(reshaped_image, (640, 480), interpolation=cv2.INTER_CUBIC)

    cv2.imshow('Temperature Image', resized_image)
    cv2.waitKey(1)

if __name__ == "__main__":
    ipcon = IPConnection() # Create IP connection
    ti = BrickletThermalImaging(UID, ipcon) # Create device object

    ipcon.connect(HOST, PORT) # Connect to brickd
    # Don't use device before ipcon is connected

    # Register temperature image callback to function cb_temperature_image
    ti.register_callback(ti.CALLBACK_TEMPERATURE_IMAGE, cb_temperature_image)

    # Enable temperature image transfer for callback
    ti.set_image_transfer_config(ti.IMAGE_TRANSFER_CALLBACK_TEMPERATURE_IMAGE)

    input("Press key to exit\n") # Use raw_input() in Python 2
    ipcon.disconnect()
    cv2.destroyAllWindows()
