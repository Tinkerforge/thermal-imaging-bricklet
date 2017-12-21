#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""
Starter Kit: Camera Slider Demo
Copyright (C) 2015 Matthias Bolte <matthias@tinkerforge.com>

main.py: Demo for Starter Kit: Camera Slider

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

# FIXME: the Stepper Brick position is represented as an int32_t. the position
# can overflow between the minimum and maximum position. the code does not deal
# with this (yet)

import sip
sip.setapi('QString', 2)
sip.setapi('QVariant', 2)

import os
import sys


import time
import signal
import subprocess
import threading
from datetime import datetime

from PyQt4.QtCore import pyqtSignal, Qt, QObject, QTimer, QEvent
from PyQt4.QtGui import QApplication, QMainWindow, QIcon, QMessageBox, QStyle, \
                        QStyleOptionSlider, QSlider, QFont

from tinkerforge.ip_connection import IPConnection
from tinkerforge.bricklet_thermal_imaging import BrickletThermalImaging
from tinkerforge.bricklet_barometer import BrickletBarometer

from ui_mainwindow import Ui_MainWindow

class MainWindow(QMainWindow, Ui_MainWindow):
    qtcb_ipcon_enumerate = pyqtSignal(str, str, 'char', type((0,)), type((0,)), int, int)
    qtcb_ipcon_connected = pyqtSignal(int)

    def __init__(self, parent=None):
        QMainWindow.__init__(self, parent)

        self.setupUi(self)
        self.setWindowTitle('Tinkerforge CES Demo: Thermal Imaging Bricklet by FLIR')

        self.qtcb_ipcon_enumerate.connect(self.cb_ipcon_enumerate)
        self.qtcb_ipcon_connected.connect(self.cb_ipcon_connected)

        # create and setup ipcon
        self.ipcon = IPConnection()
        self.ipcon.register_callback(IPConnection.CALLBACK_ENUMERATE,
                                     self.qtcb_ipcon_enumerate.emit)
        self.ipcon.register_callback(IPConnection.CALLBACK_CONNECTED,
                                     self.qtcb_ipcon_connected.emit)

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
                    if connected_uid != '0':
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

def main():

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
