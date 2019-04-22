import matplotlib.pyplot as plt
import numpy as np
import scipy.fftpack
import pandas as pd
import matplotlib.animation as animation

data = pd.read_csv("data_2/sweep_inlet_2.csv")
fig2 = plt.figure()

Fs = 1000.0;  # sampling rate
Ts = 1.0/Fs; # sampling interval

ims = []
for i in range(1,66):
    start = (i-1)*1000
    end = i*1000
    t = data.iloc[start:end,0]
    y = data.iloc[start:end,1]
    pin = data.iloc[start:end,4]

    n = len(y) # length of the signal
    k = np.arange(n)
    T = n/Fs
    frq = k/T # two sides frequency range
    frq = frq[range(int(n/2))] # one side frequency range

    Y = np.fft.fft(y)/n # fft computing and normalization
    Y = Y[range(int(n/2))]

    p, = plt.plot(frq,abs(Y),'r') # plotting the spectrum
    # plt.set_xlabel('Freq (Hz)')
    # plt.set_ylabel('|Y(freq)|')
    ims.append((p,))

im_ani = animation.ArtistAnimation(fig2, ims, interval=200, repeat_delay=2000,
                                   blit=True)

Writer = animation.writers['ffmpeg']
writer = Writer(fps=15, metadata=dict(artist='Me'), bitrate=1800)
# # To save this second animation with some metadata, use the following command:
im_ani.save('im.mp4', writer=writer)

plt.show()