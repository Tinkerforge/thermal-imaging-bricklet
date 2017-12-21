#!/bin/sh
# Connects to localhost:4223 by default, use --host and --port to change this

# FIXME: This example is incomplete

uid=XYZ # Change XYZ to the UID of your Thermal Imaging Bricklet

# Handle incoming high contrast image callbacks
tinkerforge dispatch thermal-imaging-bricklet $uid high-contrast-image &

# Enable high contrast image transfer for callback
tinkerforge call thermal-imaging-bricklet $uid set-image-transfer-config image-transfer-callback-high-contrast-image

echo "Press key to exit"; read dummy

kill -- -$$ # Stop callback dispatch in background
