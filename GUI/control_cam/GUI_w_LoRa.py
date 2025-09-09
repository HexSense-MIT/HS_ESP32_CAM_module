#!/usr/bin/env python
# work with MPU6050_kalman.ino

from PyQt5 import QtCore, QtWidgets, uic, QtGui
from pyqtgraph import PlotWidget
from PyQt5.QtWidgets import QApplication, QVBoxLayout
import pyqtgraph as pg
import numpy as np
import datetime
import serial
import sys
import os
import time
from time import sleep
from colorama import Fore, Back, Style
import csv
from matplotlib.backends.backend_qt5agg import FigureCanvasQTAgg as FigureCanvas
from matplotlib.backends.backend_qt5agg import NavigationToolbar2QT as NavigationToolbar
import matplotlib.pyplot as plt
import random
import struct
import os.path
from binascii import hexlify
import threading
import subprocess
from cobs import cobs

recv_data_cnt = 16

def read_current_time():
    now = datetime.datetime.now(datetime.timezone.utc)
    current_time = now.strftime("%Z:%j/%H:%M:%S   ")
    return current_time

class MainWindow(QtWidgets.QMainWindow):

    def __init__(self, *args, **kwargs):
        super(MainWindow, self).__init__(*args, **kwargs)
        self.setFixedSize(569, 600)

        #Load the UI Page
        uic.loadUi('Temperature_monitoring.ui', self)

        self.serial_ports_list = []
        self.serial_speed = [250000]
        self.camera_list = ['1', '2', '3', '4', '5', '6', 'ALL']
        self.cmd = [0xAA, 0x00, 0x00, 0x00, 0x00]

        # Ref: https://stackoverflow.com/questions/59898215/break-an-infinit-loop-when-button-is-pressed
        self.timer = QtCore.QTimer(self, interval=5, timeout=self.read_port)
        self.ser=serial.Serial()

        self.clear_btn.clicked.connect(self.clear_plot)
        self.close_btn.clicked.connect(self.close)

        self.scan_btn.clicked.connect(self.scan)
        self.open_btn.clicked.connect(self.open_port)
        # self.start_serial_btn.clicked.connect(self.start_read_port)
        # self.stop_serial_btn.clicked.connect(self.stop_read_port)

        self.turn_on_btn.clicked.connect(self.turn_on)
        self.turn_off_btn.clicked.connect(self.turn_off)
        self.take_photo_btn.clicked.connect(self.take_photo)
        self.grab_data_btn.clicked.connect(self.grab_data)

        self.image_bar.setValue(0)

        for x in self.serial_speed:
            self.speed_comboBox.addItem(str(x))
        for x in self.camera_list:
            self.camera_comboBox.addItem(str(x))

        self.image_size = 0

    def scan(self):
        if os.name == 'nt':  # sys.platform == 'win32':
            from serial.tools.list_ports_windows import comports
        elif os.name == 'posix':
            from serial.tools.list_ports_posix import comports

        for info in comports(False):
            port, desc, hwid = info
        iterator = sorted(comports(False))

        self.serial_ports_list = [] # clear the list first
        for n, (port, desc, hwid) in enumerate(iterator, 1):
            self.serial_ports_list.append("{:20} ".format(port))

        self.serial_comboBox.clear() # clear the list first
        for x in self.serial_ports_list:
            self.serial_comboBox.addItem(x)

        self.start_id = 0

        self.waveform_color = 'b'

    def open_port(self):
        index = self.serial_comboBox.currentIndex()
        serial_ports_port = self.serial_ports_list[index][:-1] # delete the \n at the end
        index = self.speed_comboBox.currentIndex()
        self.ser = serial.Serial(serial_ports_port, self.serial_speed[index])

        current_time = read_current_time()
        self.log.append(current_time + self.ser.name + " Opened @ " + str(self.serial_speed[index]) + "bps")

    # def start_read_port(self):
    #     self.timer.start() # Start monitoring the serialport
    #     current_time = read_current_time()
    #     self.log.append(current_time + " :  Start monitoring the Serial Port...")
    #
    # def stop_read_port(self):
    #     current_time = read_current_time()
    #     self.log.append(current_time + " :  Stop monitoring the Serial Port.\n")
    #     self.timer.stop() # Stop the timer

    def get_cam_num(self):
        # get camera_comboBox current index
        index = self.camera_comboBox.currentIndex()
        self.cmd = [0xAA, index, 0x00, 0x00, 0x00]

    def turn_on(self):
        self.get_cam_num()
        self.cmd[2] = 0x01
        cmd_byte = bytearray(self.cmd)
        # print("cmd sent: ", cmd_byte)
        encoded_cmd = cobs.encode(cmd_byte) + b'\x00'
        self.ser.write(encoded_cmd)

        i = 0
        while(not self.ser.inWaiting()):
            sleep(0.1)
            i = i + 1
            if (i > 20):
                print("No response from the HexSense!")
                break

        reply = []
        while (self.ser.inWaiting()):
            recv_data = self.ser.read(1)
            reply.append(recv_data)
        print("reply: ", reply)

        cam_num = int.from_bytes(reply[1], "little") + 1

        if (len(reply) == 9 and reply[0] == b'\xaa'):
            current_time = read_current_time()
            self.log.append(current_time + " :  Camera #" + str(cam_num) + " is turned ON.")
            sleep(1)
            self.log.append(current_time + " :  Camera #" + str(cam_num) + " is READY.")

    def turn_off(self):
        self.get_cam_num()
        self.cmd[2] = 0x02
        cmd_byte = bytearray(self.cmd)
        # print("cmd sent: ", cmd_byte)
        encoded_cmd = cobs.encode(cmd_byte) + b'\x00'
        self.ser.write(encoded_cmd)

        i = 0
        while (not self.ser.inWaiting()):
            sleep(0.1)
            i = i + 1
            if (i > 20):
                print("No response from the HexSense!")
                break

        reply = []
        while (self.ser.inWaiting()):
            recv_data = self.ser.read(1)
            reply.append(recv_data)
        print("reply: ", reply)

        cam_num = int.from_bytes(reply[1], "little") + 1

        if (len(reply) == 9 and reply[0] == b'\xaa'):
            current_time = read_current_time()
            self.log.append(current_time + " :  Camera #" + str(cam_num) + " is turned OFF.")

    def take_photo(self):
        self.get_cam_num()
        self.cmd[2] = 0x03
        cmd_byte = bytearray(self.cmd)
        # print("cmd sent: ", cmd_byte)
        encoded_cmd = cobs.encode(cmd_byte) + b'\x00'
        self.ser.write(encoded_cmd)

        i = 0
        while (not self.ser.inWaiting()):
            sleep(0.1)
            i = i + 1
            if (i > 20):
                print("No response from the HexSense!")
                break

        reply = []
        reply_list = []
        while (self.ser.inWaiting()):
            recv_data = self.ser.read(1)
            # print(recv_data.hex(), end='', flush=True)
            reply_list.append(recv_data)
            reply.append(recv_data[0])

        self.image_size = reply[1] + (reply[2] << 8) + (reply[3] << 16) + (reply[4] << 24)
        cam_num = self.cmd[1] + 1
        self.image_bar.setMaximum(self.image_size)

        if (len(reply) == 9 and reply[0] == 170):
            current_time = read_current_time()
            self.log.append(current_time + " :  Image size from #" + str(cam_num) + " - " + str(self.image_size) + " bytes")

    def parse_image(self, image_data):
        with open("output_image.jpg", "wb") as image_file:
            image_file.write(image_data)

    def grab_data(self):
        self.get_cam_num()
        self.cmd[2] = 0x04
        cmd_byte = bytearray(self.cmd)
        # print("cmd sent: ", cmd_byte)
        encoded_cmd = cobs.encode(cmd_byte) + b'\x00'
        self.ser.write(encoded_cmd)

        image_data = bytearray() # to store the image data

        i = 0
        while (not self.ser.inWaiting()):
            sleep(0.1)
            i = i + 1
            if (i > 20):
                print("No response from the HexSense!")
                break

        byte_i = 0
        while (byte_i < self.image_size):
            if (self.ser.inWaiting()):
                recv_data = self.ser.read(1)
                # print(recv_data.hex(), end='', flush=True)
                image_data.append(recv_data[0])
                byte_i += 1
                print(f'\rProgress: {byte_i/self.image_size*100:.1f}%', end='', flush=True)
                # print(byte_i, self.image_size)
                # self.image_bar.setValue(byte_i)

        cam_num = self.cmd[1] + 1
        current_time = read_current_time()
        self.log.append(current_time + " :  received image from Camera #" + str(cam_num))
        self.parse_image(image_data)
        self.log.append(current_time + " :  write image to output_image.jpg")

    def read_port(self):
        if (self.ser.inWaiting()):
            current_time = read_current_time()
            recv_data = self.ser.read(recv_data_cnt)
            print("recv_data: ", recv_data)

            rtd_1_temp_i = recv_data[0:4]
            rtd_1_temp_d = int.from_bytes(rtd_1_temp_i, "little")

            rtd_2_temp_i = recv_data[4:8]
            rtd_2_temp_d = int.from_bytes(rtd_2_temp_i, "little")

            rtd_3_temp_i = recv_data[8:12]
            rtd_3_temp_d = int.from_bytes(rtd_3_temp_i, "little")

            rtd_4_temp_i = recv_data[12:16]
            rtd_4_temp_d = int.from_bytes(rtd_4_temp_i, "little")

            # print(rtd_1_temp_d, rtd_2_temp_d, rtd_3_temp_d, rtd_4_temp_d)
            # show only two digits
            rtd_1_temp_d = round(rtd_1_temp_d, 2)
            rtd_2_temp_d = round(rtd_2_temp_d, 2)
            rtd_3_temp_d = round(rtd_3_temp_d, 2)
            rtd_4_temp_d = round(rtd_4_temp_d, 2)

            self.log.append(current_time + " -> RTD1: " + str(rtd_1_temp_d) + " | RTD2: " + str(rtd_2_temp_d) +
                        " | RTD3: " + str(rtd_3_temp_d) + " | RTD4: " + str(rtd_4_temp_d))

            self.RTD_1_temp.pop(0)
            self.RTD_1_temp.append(rtd_1_temp_d)

            self.RTD_2_temp.pop(0)
            self.RTD_2_temp.append(rtd_2_temp_d)

            self.RTD_3_temp.pop(0)
            self.RTD_3_temp.append(rtd_3_temp_d)

            self.RTD_4_temp.pop(0)
            self.RTD_4_temp.append(rtd_4_temp_d)

            self.rtd1_plot.clear()
            self.rtd2_plot.clear()
            self.rtd3_plot.clear()
            self.rtd4_plot.clear()

            self.rtd1_plot.plot(self.time_index, self.RTD_1_temp, pen=pg.mkPen('b', width=3))
            self.rtd1_plot.plot(self.time_index, self.RTD_2_temp, pen=pg.mkPen('r', width=3))
            self.rtd1_plot.plot(self.time_index, self.RTD_3_temp, pen=pg.mkPen('g', width=3))
            self.rtd1_plot.plot(self.time_index, self.RTD_4_temp, pen=pg.mkPen('k', width=3))
            # self.rtd2_plot.plot(self.time_index, self.RTD_2_temp, pen=pg.mkPen('r', width=3))
            # self.rtd3_plot.plot(self.time_index, self.RTD_3_temp, pen=pg.mkPen('k', width=3))
            # self.rtd4_plot.plot(self.time_index, self.RTD_4_temp, pen=pg.mkPen('k', width=3))

            if (self.rec_mode == 0):
                if self.recording == 1:
                    self.file.write(recv_data)
                    self.waveform_color = 'r'

                if self.recording == 0:
                    self.file.close()
                    self.waveform_color = 'b'

            if (self.rec_mode == 1):
                if self.recording == 1:
                    self.file.write(recv_data)
                    self.recording_cnt += 1
                    self.waveform_color = 'r'

                    self.image_bar.setValue(self.recording_cnt)

                if self.recording_cnt == self.REC_DATA_LEN:
                    self.recording = 0
                    self.log.append(current_time + " -------> Stop automatic recording data. <-------")
                    self.recording_cnt = 0

                if self.recording == 0:
                    self.file.close()
                    self.waveform_color = 'b'

    def clear_plot(self):
        self.log.clear()

# driver code 
if __name__ == '__main__': 
    # creating apyqt5 application 
    app = QApplication(sys.argv) 
    # creating a window object 
    main = MainWindow() 
    # showing the window 
    main.show()
    # loop 
    sys.exit(app.exec_()) 
