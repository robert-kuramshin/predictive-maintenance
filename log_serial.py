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

# plot_window = 20
# y_var = np.array(np.zeros([plot_window]))

# plt.ion()
# fig, ax = plt.subplots()
# line, = ax.plot(y_var)

while True:
    try:
        ser_bytes = ser.readline()


        # try:
            
        # except:
        #     continue

        print(ser_bytes)
        with open("test.csv","a") as f:
            f.write(ser_bytes)
        # y_var = np.append(y_var,decoded_bytes)
        # y_var = y_var[1:plot_window+1]
        # line.set_ydata(y_var)
        # ax.relim()
        # ax.autoscale_view()
        # fig.canvas.draw()
        # fig.canvas.flush_events()
    except:
        print("Keyboard Interrupt")
        break