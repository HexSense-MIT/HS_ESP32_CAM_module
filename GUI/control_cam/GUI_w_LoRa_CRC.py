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
import math
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
chunk_size    = 20

def read_current_time():
    now = datetime.datetime.now(datetime.timezone.utc)
    current_time = now.strftime("%Z:%j/%H:%M:%S   ")
    return current_time

class MainWindow(QtWidgets.QMainWindow):

    def __init__(self, *args, **kwargs):
        super(MainWindow, self).__init__(*args, **kwargs)
        self.setFixedSize(569, 600)

        #Load the UI Page
        uic.loadUi('GUI_LoRa_w_CRC.ui', self)

        self.serial_ports_list = []
        self.serial_speed = [250000]
        self.camera_list = ['1', '2', '3', '4', '5', '6', 'ALL']
        self.cmd = [0xAA, 0x00, 0x00, 0x00, 0x00]

        # Ref: https://stackoverflow.com/questions/59898215/break-an-infinit-loop-when-button-is-pressed
        # self.timer = QtCore.QTimer(self, interval=5, timeout=self.read_port)
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
        self.get_data_btn.clicked.connect(self.get_data)

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

        # decode the reply
        reply = b''.join(reply)
        reply = cobs.decode(reply[:-1]) # remove the last 0x00
        # print("reply: ", reply)

        cam_num = int.from_bytes(bytes([reply[1]]), "little") + 1

        if (len(reply) == 7 and reply[0] == 170):
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

        # decode the reply
        reply = b''.join(reply)
        reply = cobs.decode(reply[:-1])  # remove the last 0x00
        # print("reply: ", reply)

        cam_num = int.from_bytes(bytes([reply[1]]), "little") + 1

        if (len(reply) == 7 and reply[0] == 170):
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

        reply_list = []
        while (self.ser.inWaiting()):
            recv_data = self.ser.read(1)
            # print(recv_data.hex(), end='', flush=True)
            reply_list.append(recv_data)

        # decode the reply
        reply_list = b''.join(reply_list)
        reply_list = cobs.decode(reply_list[:-1])  # remove the last 0x00
        # print("reply: ", reply_list)

        self.image_size = int.from_bytes(bytes([reply_list[3], reply_list[4], reply_list[5], reply_list[6]]), "little")
        cam_num = self.cmd[1] + 1
        self.image_bar.setMaximum(self.image_size)

        if (len(reply_list) == 7 and reply_list[0] == 170):
            current_time = read_current_time()
            self.log.append(current_time + " :  Image size from #" + str(cam_num) + " : " + str(self.image_size) + " bytes")

    def parse_image(self, image_data):
        with open("output_image_LoRa.jpg", "wb") as image_file:
            image_file.write(image_data)

    def grab_data(self):
        self.get_cam_num()
        self.cmd[2] = 0x04
        cmd_byte = bytearray(self.cmd)
        encoded_cmd = cobs.encode(cmd_byte) + b'\x00'
        self.ser.write(encoded_cmd)

        i = 0
        while not self.ser.inWaiting():
            sleep(1)
            print(".", end='', flush=True)
            i += 1
            if i > 60:  # wait for 60 seconds
                print("No response from the HexSense!")
                break

        reply_list = []
        while (self.ser.inWaiting()):
            recv_data = self.ser.read(1)
            reply_list.append(recv_data)

        # decode the reply
        reply_list = b''.join(reply_list)
        reply_list = cobs.decode(reply_list[:-1])  # remove the last 0x00

        if (len(reply_list) == 7 and reply_list[0] == 170):
            current_time = read_current_time()
            self.log.append(current_time + "stored image data in buffer.")

    def get_data(self):
        self.get_cam_num()
        self.cmd[2] = 0x05  # NEXT_PACKET_CAM_CODE
        cmd_byte = bytearray(self.cmd)
        encoded_cmd = cobs.encode(cmd_byte) + b'\x00'
        self.ser.write(encoded_cmd)

        image_data = bytearray()
        bytes_received = 0
        seq_num = 0

        print("\nStarting image transfer...")
        self.image_bar.setValue(0)

        while bytes_received < self.image_size:
            # --- read one full packet ---
            packet = bytearray()
            while len(packet) < (chunk_size + 4):  # header(3) + data(≤20) + checksum(1)
                if self.ser.inWaiting():
                    packet.append(self.ser.read(1)[0])

            # --- verify checksum ---
            checksum = sum(packet[:-1]) & 0xFF
            if checksum != packet[-1]:
                # bad packet → request retransmission until correct
                print(f"\nChecksum failed at seq {seq_num}, requesting retransmit...")
                self.cmd[2] = 0x06  # RETRANSMIT_CAM_CODE
                encoded_cmd = cobs.encode(bytearray(self.cmd)) + b'\x00'
                self.ser.write(encoded_cmd)
                continue

            # --- valid packet ---
            cam_num = packet[1]
            recv_seq = packet[2]
            payload = packet[3:-1]

            if recv_seq != seq_num:
                print(f"\nWarning: expected seq {seq_num}, got {recv_seq}")
                seq_num = recv_seq  # resync

            image_data.extend(payload)
            bytes_received += len(payload)
            seq_num += 1
            if seq_num > 255:
                seq_num = 0

            self.image_bar.setValue(bytes_received)
            print(f'\rProgress: {bytes_received / self.image_size * 100:.2f}%', end='', flush=True)

            # ask next packet if not finished
            if bytes_received < self.image_size:
                self.cmd[2] = 0x05  # NEXT_PACKET_CAM_CODE
                encoded_cmd = cobs.encode(bytearray(self.cmd)) + b'\x00'
                self.ser.write(encoded_cmd)

        print("\nImage transfer complete.")
        self.parse_image(image_data)  # save as output_image_LoRa.jpg

        cam_num = self.cmd[1] + 1
        current_time = read_current_time()
        self.log.append(current_time + f" :  received {bytes_received} bytes from Camera #{cam_num}")
        self.log.append(current_time + " :  saved as output_image_LoRa.jpg")

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
