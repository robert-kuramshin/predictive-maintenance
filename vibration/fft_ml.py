import matplotlib.pyplot as plt
import numpy as np
import scipy.fftpack
import pandas as pd
import matplotlib.animation as animation

from sklearn.ensemble import GradientBoostingRegressor
from sklearn.model_selection import GridSearchCV
from sklearn.metrics import mean_squared_error

train = pd.read_csv("../data_2/train.csv")
test = pd.read_csv("../data_2/test.csv")

Fs = 1000.0;  # sampling rate
Ts = 1.0/Fs; # sampling interval

#Train
X_train = []
y_train = []
for i in range(1,int(train.shape[0]/1000)):
    start = (i-1)*1000
    end = i*1000
    t = train.iloc[start:end,0] #time
    y = train.iloc[start:end,1] #vibration
    pin = train.iloc[start:end,5] #pressure inlet

    pin = pin.mean()

    n = len(y) # length of the signal
    k = np.arange(n)
    T = n/Fs
    frq = k/T # two sides frequency range
    frq = frq[range(int(n/2))] # one side frequency range

    Y = np.fft.fft(y)/n # fft computing and normalization
    Y = Y[range(int(n/2))]
    
    X_train.append(abs(Y))
    y_train.append(pin)

#Test
X_test = []
y_test = []
for i in range(1,int(test.shape[0]/1000)):
    start = (i-1)*1000
    end = i*1000
    t = test.iloc[start:end,0] #time
    y = test.iloc[start:end,1] #vibration
    pin = test.iloc[start:end,5] #pressure inlet

    pin = pin.mean()

    n = len(y) # length of the signal
    k = np.arange(n)
    T = n/Fs
    frq = k/T # two sides frequency range
    frq = frq[range(int(n/2))] # one side frequency range

    Y = np.fft.fft(y)/n # fft computing and normalization
    Y = Y[range(int(n/2))]
    
    X_test.append(abs(Y))
    y_test.append(pin)

print(np.array(X_train).shape)
print(np.array(y_train).shape)
print(np.array(X_test).shape)
print(np.array(y_test).shape)

# tuned_parameters = [{'n_estimators': [500],
#                      'min_samples_leaf':[2,3,4],
#                     'min_samples_split': [0.75,1.0,2],
#                     'learning_rate': [0.05,0.1],
#                 'loss': ['ls']}]

# clf = GridSearchCV(GradientBoostingRegressor(), tuned_parameters, cv=5,n_jobs=4)
# clf.fit(X_train, y_train)

# print("Best parameters set found on development set:")
# print()
# print(clf.best_params_)

# reg = GradientBoostingRegressor(**clf.best_params_)

params = {'n_estimators': 500, 'min_samples_leaf': 2, 'loss': 'ls', 'learning_rate': 0.05, 'min_samples_split': 1.0}
reg = GradientBoostingRegressor(**params)

reg.fit(X_train, y_train)
predicted = reg.predict(X_test)
t = range(1,int(test.shape[0]/1000))

mse = mean_squared_error(y_test, reg.predict(X_test))
print("MSE: %.4f" % mse)

df=pd.DataFrame({'x': t, 'real_y': y_test, 'predicted':predicted })

plt.plot( 'x', 'real_y', data=df, marker='', color='blue', linewidth=2, label="inlet pressure")
plt.plot( 'x', 'predicted', data=df, marker='', color='red', linewidth=2, label="vibration response")
plt.legend()
plt.show()