#!/usr/bin/env ruby
# -*- ruby encoding: utf-8 -*-

# FIXME: This example is incomplete

require 'tinkerforge/ip_connection'
require 'tinkerforge/bricklet_thermal_imaging'

include Tinkerforge

HOST = 'localhost'
PORT = 4223
UID = 'XYZ' # Change XYZ to the UID of your Thermal Imaging Bricklet

ipcon = IPConnection.new # Create IP connection
ti = BrickletThermalImaging.new UID, ipcon # Create device object

ipcon.connect HOST, PORT # Connect to brickd
# Don't use device before ipcon is connected

# Register high contrast image callback
ti.register_callback(BrickletThermalImaging::CALLBACK_HIGH_CONTRAST_IMAGE) do |image|
  puts "Image: #{image}"
end

# Enable high contrast image transfer for callback
ti.set_image_transfer_config BrickletThermalImaging::IMAGE_TRANSFER_CALLBACK_HIGH_CONTRAST_IMAGE

puts 'Press key to exit'
$stdin.gets
ipcon.disconnect
