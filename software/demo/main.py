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

    image_pixel_width = 1
    crosshair_width = 1

    def __init__(self, parent=None):
        QMainWindow.__init__(self, parent)

        self.setupUi(self)
        self.setWindowTitle('Tinkerforge CES Demo: Thermal Imaging Bricklet by FLIR')

        self.qtcb_ipcon_enumerate.connect(self.cb_ipcon_enumerate)
        self.qtcb_ipcon_connected.connect(self.cb_ipcon_connected)
        self.qtcb_high_contrast_image.connect(self.cb_high_contrast_image)
        
        self.image = QImage(QSize(80, 60), QImage.Format_RGB32)

        # Add Tinkerforge logo
        # Adapt this path
        self.label_logo.setPixmap(QPixmap('/home/pi/Desktop/demo/logo_klein.png'))

        # create and setup ipcon
        self.ipcon = IPConnection()
        self.ipcon.connect(HOST, PORT)
        self.ipcon.register_callback(IPConnection.CALLBACK_ENUMERATE,
                                     self.qtcb_ipcon_enumerate.emit)
        self.ipcon.register_callback(IPConnection.CALLBACK_CONNECTED,
                                     self.qtcb_ipcon_connected.emit)

        self.ti = BrickletThermalImaging(UID_TI, self.ipcon)
        self.ti.register_callback(self.ti.CALLBACK_HIGH_CONTRAST_IMAGE, self.qtcb_high_contrast_image.emit)
        self.ti.set_image_transfer_config(self.ti.IMAGE_TRANSFER_CALLBACK_HIGH_CONTRAST_IMAGE)
        
        self.ti.set_spotmeter_config([35, 25, 45, 35])
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
            
            if 35 != None and 45 != None:
                pen = QPen()
                pen.setColor(Qt.white)
                pen.setWidth(0.2)
                painter.setPen(pen)

                from_x, from_y, to_x, to_y = [35, 25, 45, 35]

                from_x = from_x*self.image_pixel_width+1
                from_y = from_y*self.image_pixel_width+1
                to_x = to_x*self.image_pixel_width+1
                to_y = to_y*self.image_pixel_width+1

                cross_x = from_x + (to_x-from_x)/2.0
                cross_y = from_y + (to_y-from_y)/2.0

                if to_x-from_x > 5 or to_y - from_y > 5:
                    lines = [QLine(from_x, from_y, from_x+self.crosshair_width, from_y),
                             QLine(from_x, from_y, from_x, from_y+self.crosshair_width),
                             QLine(to_x, to_y, to_x, to_y-self.crosshair_width),
                             QLine(to_x, to_y, to_x-self.crosshair_width, to_y),
                             QLine(from_x, to_y, from_x, to_y-self.crosshair_width),
                             QLine(from_x, to_y, from_x+self.crosshair_width, to_y),
                             QLine(to_x, from_y, to_x, from_y+self.crosshair_width),
                             QLine(to_x, from_y, to_x-self.crosshair_width, from_y)]
                    painter.drawLines(lines)

                lines = [QLine(cross_x-self.crosshair_width, cross_y, cross_x+self.crosshair_width, cross_y),
                         QLine(cross_x, cross_y-self.crosshair_width, cross_x, cross_y+self.crosshair_width)]
                painter.drawLines(lines)

            self.update_spotmeter_roi_label()
        
        self.label_image.paintEvent = paintEvent

    def update_spotmeter_roi_label(self):
        from_x, from_y, to_x, to_y = self.ti.get_spotmeter_config()
        self.spotmeter_roi_label.setText('from <b>[{0}, {1}]</b> to <b>[{2}, {3}]</b>'.format(from_x, from_y, to_x, to_y))

    def cb_ipcon_enumerate(self, uid, connected_uid, position,
                           hardware_version, firmware_version,
                           device_identifier, enumeration_type):
        if self.ipcon.get_connection_state() != IPConnection.CONNECTION_STATE_CONNECTED:
            # ignore enumerate callbacks that arrived after the connection got closed
            return

    def cb_ipcon_connected(self, connect_reason):
        try:
            self.ipcon.enumerate()
        except:
            pass

    def cb_high_contrast_image(self, image):
        if image != None:
            self.new_image(image)

        print("New image")
        self.statistics(self.ti.get_statistics())

    def statistics(self, data):
        self.valid_resolution = data.resolution
        spot_mean = self.kelvin_to_degstr(data.spotmeter_statistics[0])
        spot_max = self.kelvin_to_degstr(data.spotmeter_statistics[1])
        spot_min = self.kelvin_to_degstr(data.spotmeter_statistics[2])
        spot_pix = str(data.spotmeter_statistics[3])
        self.spotmeter_mean_label.setText(spot_mean)
        self.spotmeter_minimum_label.setText(spot_min)
        self.spotmeter_maximum_label.setText(spot_max)
        self.spotmeter_pixel_count_label.setText(spot_pix)

        #temp_fpa = self.kelvin_to_degstr(data.temperatures[0], 1)
        #temp_fpa_ffc = self.kelvin_to_degstr(data.temperatures[1], 1)
        #temp_housing = self.kelvin_to_degstr(data.temperatures[2], 1)
        #temp_housing_ffc = self.kelvin_to_degstr(data.temperatures[3], 1)
        #self.temp_fpa_label.setText(temp_fpa)
        #self.temp_fpa_ffc_label.setText(temp_fpa_ffc)
        #self.temp_housing_label.setText(temp_housing)
        #self.temp_housing_ffc_label.setText(temp_housing_ffc)

        #sheet_green  = "QLabel { color: green; }"
        #sheet_orange = "QLabel { color: orange; }"
        #sheet_red    = "QLabel { color: red; }"

        #if data.ffc_status == 0b00:
        #    self.ffc_state_label.setStyleSheet(sheet_orange)
        #    self.ffc_state_label.setText('Never Commanded')
        #elif data.ffc_status == 0b01:
        #    self.ffc_state_label.setStyleSheet(sheet_orange)
        #    self.ffc_state_label.setText('Imminent')
        #elif data.ffc_status == 0b10:
        #    self.ffc_state_label.setStyleSheet(sheet_orange)
        #    self.ffc_state_label.setText('In Progress')
        #elif data.ffc_status == 0b11:
        #    self.ffc_state_label.setStyleSheet(sheet_green)
        #    self.ffc_state_label.setText('Complete')

        #if data.temperature_warning[0]:
        #    self.shutter_lockout_label.setStyleSheet(sheet_red)
        #    self.shutter_lockout_label.setText('Yes')
        #else:
        #    self.shutter_lockout_label.setStyleSheet(sheet_green)
        #    self.shutter_lockout_label.setText('No')

        #if data.temperature_warning[1]:
        #    self.overtemp_label.setStyleSheet(sheet_red)
        #    self.overtemp_label.setText('Yes')
        #else:
        #    self.overtemp_label.setStyleSheet(sheet_green)
        #    self.overtemp_label.setText('No')

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

    def kelvin_to_degstr(self, value, res = None):
        if res == None:
            res = self.valid_resolution
        if res == 0:
            return "{0:.2f}".format(value/10.0 - 273.15)
        else:
            return "{0:.2f}".format(value/100.0 - 273.15)

def main():

    os.system('xset s off') # disable screensaver
    os.system('xset -dpms') # disable display power management 
    args = sys.argv

    application = QApplication(args)

    main_window = MainWindow()

    #Show as fullscreen
    if '-fullscreen' in sys.argv:
        main_window.showFullScreen()
    else:
        main_window.show()

    sys.exit(application.exec_())

if __name__ == "__main__":
    main()
