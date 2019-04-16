import serial
import time
import csv
import matplotlib
matplotlib.use("tkAgg")
import matplotlib.pyplot as plt
import numpy as np

ser = serial.Serial('/dev/ttyUSB0')
ser.baudrate = 2000000
ser.flushInput()

while True:
    try:
        ser_bytes = ser.readline()

        print(ser_bytes)
        with open("test.csv","a") as f:
            f.write(ser_bytes)
    except:
        print("Keyboard Interrupt")
        break