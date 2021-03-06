import matplotlib.pyplot as plt
import numpy as np
import scipy.fftpack
import pandas as pd
import matplotlib.animation as animation

#TRAIN
train_data = pd.read_csv("../data_2/dataset.csv")

#TEST
test_data = pd.read_csv("../data_2/sweep_inlet.csv")

train_data.to_csv("../data_2/train.csv")
test_data.to_csv("../data_2/test.csv")