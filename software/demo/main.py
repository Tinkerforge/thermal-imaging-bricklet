#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""
Demo Kit: CES 2018 FLIR

Copyright (C) 2017 Lukas Lauer <lukas@tinkerforge.com>

main.py: Demo for Thermal Imaging and some other sensors

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public
License along with this program; if not, write to the
Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.
"""

import sip
sip.setapi('QString', 2)
sip.setapi('QVariant', 2)

import os
import sys

import colorsys
import math


import time
import signal
import subprocess
import threading
from datetime import datetime

from PyQt4.QtCore import pyqtSignal, Qt, QObject, QTimer, QEvent, QSize, QPoint, QLine
from PyQt4.QtGui import QApplication, QMainWindow, QIcon, QMessageBox, QStyle, \
                        QStyleOptionSlider, QSlider, QFont, QWidget, QImage, QPainter, QPen, QColor, QPixmap

from tinkerforge.ip_connection import IPConnection
from tinkerforge.bricklet_thermal_imaging import BrickletThermalImaging
from tinkerforge.bricklet_barometer import BrickletBarometer
from tinkerforge.bricklet_humidity_v2 import BrickletHumidityV2
from tinkerforge.bricklet_ambient_light_v2 import BrickletAmbientLightV2

from ui_mainwindow import Ui_MainWindow

HOST = "localhost"
PORT = 4223

UID_TI = "XYZ"
UID_AL = "yxP"
UID_HU = "Djm"
UID_BM = "vMD"

class MainWindow(QMainWindow, Ui_MainWindow):
    qtcb_ipcon_enumerate = pyqtSignal(str, str, 'char', type((0,)), type((0,)), int, int)
    qtcb_ipcon_connected = pyqtSignal(int)

    qtcb_high_contrast_image = pyqtSignal(object)

    def __init__(self, parent=None):
        QMainWindow.__init__(self, parent)

        self.setupUi(self)
        self.setWindowTitle('Tinkerforge CES Demo: Thermal Imaging Bricklet by FLIR')

        self.qtcb_ipcon_enumerate.connect(self.cb_ipcon_enumerate)
        self.qtcb_ipcon_connected.connect(self.cb_ipcon_connected)
        self.qtcb_high_contrast_image.connect(self.cb_high_contrast_image)
        
        self.image = QImage(QSize(80, 60), QImage.Format_RGB32)

        # Add Tinkerforge logo
        self.label_logo.setPixmap(QPixmap('logo_klein.png'))

        # create and setup ipcon
        self.ipcon = IPConnection()
        self.ipcon.connect(HOST, PORT)
        self.ipcon.register_callback(IPConnection.CALLBACK_ENUMERATE,
                                     self.qtcb_ipcon_enumerate.emit)
        self.ipcon.register_callback(IPConnection.CALLBACK_CONNECTED,
                                     self.qtcb_ipcon_connected.emit)

        ti = BrickletThermalImaging(UID_TI, self.ipcon)
        ti.register_callback(ti.CALLBACK_HIGH_CONTRAST_IMAGE, self.qtcb_high_contrast_image.emit)
        ti.set_image_transfer_config(ti.IMAGE_TRANSFER_CALLBACK_HIGH_CONTRAST_IMAGE)
        
        ti.set_spotmeter_config([35, 25, 45, 35])
        #print(ti.get_spotmeter_config())


        al = BrickletAmbientLightV2(UID_AL, self.ipcon)
        al.register_callback(al.CALLBACK_ILLUMINANCE, self.cb_illuminance)
        al.set_illuminance_callback_period(500)
        
        hu = BrickletHumidityV2(UID_HU, self.ipcon)
        hu.register_callback(hu.CALLBACK_HUMIDITY, self.cb_humidity)
        hu.set_humidity_callback_configuration(500, False, "x", 0, 0)

        bm = BrickletBarometer(UID_BM, self.ipcon)
        bm.register_callback(bm.CALLBACK_AIR_PRESSURE, self.cb_air_pressure)
        bm.set_air_pressure_callback_period(500)
        
        self.rgb_lookup = []
        for x in range(256):
            x /= 255.0
            r = int(round(255*math.sqrt(x)))
            g = int(round(255*pow(x,3)))
            if math.sin(2 * math.pi * x) >= 0:
                b = int(round(255*math.sin(2 * math.pi * x)))
            else:
                b = 0
            self.rgb_lookup.append((r, g, b))
        
        def paintEvent(event):
            painter = QPainter(self.label_image)
            
            painter.setRenderHint(QPainter.SmoothPixmapTransform, True)
            
            w = self.label_image.size().width()
            h = self.label_image.size().height()
            
            painter.scale(w / 80.0, h / 60.0)
            painter.drawImage(0, 0, self.image)
        
        self.label_image.paintEvent = paintEvent

        #self.qtcb_high_contrast_image.connect(self.cb_high_contrast_image)

    def cb_ipcon_enumerate(self, uid, connected_uid, position,
                           hardware_version, firmware_version,
                           device_identifier, enumeration_type):
        if self.ipcon.get_connection_state() != IPConnection.CONNECTION_STATE_CONNECTED:
            # ignore enumerate callbacks that arrived after the connection got closed
            return

        """if enumeration_type in [IPConnection.ENUMERATION_TYPE_AVAILABLE,
                                IPConnection.ENUMERATION_TYPE_CONNECTED]:
            self.devices[uid] = (connected_uid, device_identifier)

            def add_item(combo, no_device_text):
                if combo.itemText(0) == no_device_text:
                    combo.clear()

                if combo.findData(uid) < 0:
                    if conn0,6ected_uid != '0':
                        if connected_uid in self.devices and self.devices[connected_uid][1] in DEVICE_IDENTIFIERS:
                            connected_name = DEVICE_IDENTIFIERS[self.devices[connected_uid][1]]
                        else:
                            connected_name = 'Unknown Brick'

                        text = '{0} @ {1} [{2}]'.format(uid, connected_name, connected_uid)
                    else:
                        text = uid

                    combo.addItem(text, uid)

            if device_identifier == BrickStepper.DEVICE_IDENTIFIER:
                add_item(self.combo_stepper_uid, NO_STEPPER_BRICK_FOUND)
            elif device_identifier == BrickletIO4.DEVICE_IDENTIFIER:
                add_item(self.combo_io4_uid, NO_IO4_BRICKLET_FOUND)

            if str(device_identifier).startswith('1'):
                def update_items(combo, expected_device_identifier):
                    for other_uid in self.devices:
                        other_connected_uid, other_device_identifier = self.devices[other_uid]

                        if other_device_identifier != expected_device_identifier:
                            continue

                        if other_connected_uid == uid and device_identifier in DEVICE_IDENTIFIERS:
                            connected_name = DEVICE_IDENTIFIERS[device_identifier]
                            text = '{0} @ {1} [{2}]'.format(other_uid, connected_name, uid)
                            index = combo.findData(other_uid)

                            if index >= 0:
                                combo.setItemText(index, text)

                update_items(self.combo_stepper_uid, BrickStepper.DEVICE_IDENTIFIER)
                update_items(self.combo_io4_uid, BrickletIO4.DEVICE_IDENTIFIER)"""

    def cb_ipcon_connected(self, connect_reason):
        try:
            self.ipcon.enumerate()
        except:
            pass

    def cb_high_contrast_image(self, image):
        if image != None:
            self.new_image(image)

    def cb_humidity(self, humidity):
        #print("Humidity: " + str(humidity/100.0) + " %RH")
        self.label_humidity.setText(str(humidity/100) + " %RH")

    def cb_illuminance(self, illuminance):
        #print("Illuminance: " + str(illuminance/100.0) + " Lux")
        self.label_brightness.setText(str(illuminance/100) + " Lux")

    def cb_air_pressure(self, air_pressure):
        #print("Air Pressure: " + str(air_pressure/1000.0) + " mbar")
        self.label_airpressure.setText(str(air_pressure/1000) + " mbar")

    def new_image(self, image):

        for i, value in enumerate(image):
            r, g, b = self.rgb_lookup[value]
            self.image.setPixel(QPoint(i%80, i//80), (r << 16) | (g << 8) | b)
        self.label_image.update()
        #self.label_image.setPixmap(QPixmap(self.image))

def main():

    os.system('xset s off') # disable screensaver
    os.system('xset -dpms') # disable display power management 
    args = sys.argv

    application = QApplication(args)

    main_window = MainWindow()

    if '-fullscreen' in sys.argv:
        main_window.showFullScreen()
    else:
        main_window.show()

    sys.exit(application.exec_())

if __name__ == "__main__":
    main()
