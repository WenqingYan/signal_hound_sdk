# local and remote is what adversary could listen
# request and response is what legitimate user could listen
# raw data frequency is 0.5s
import pandas as pd
from io import StringIO
import matplotlib.pyplot as plt
import numpy as np
from numpy import NaN
import math
from scipy.signal import find_peaks


def readfile(filename):
    types = {'frequency': float,'power': float}
    df = pd.read_csv(filename, skiprows=0, header=None,  warn_bad_lines=True, delim_whitespace=False, delimiter=',')
    df.columns = ['frequency', 'power']
    df.dropna(inplace=True)
    #print(df.describe())
    return df


def frange(start, end=None, inc=None):
    "A range function, that does accept float increments..."

    if end == None:
        end = start + 0.0
        start = 0.0

    if inc == None:
        inc = 1.0

    L = []
    while 1:
        next = start + len(L) * inc
        if inc > 0 and next >= end:
            break
        elif inc < 0 and next <= end:
            break
        L.append(next)

    return L

def downsampling(df, rawf=200, tarf=100):
    downFactor = rawf//tarf
    index = [x for x in df.index.values if (x % downFactor == 0)]
    df_new = df.iloc[index,:]
    return df_new


# change the time series to the supervised learning sequence
def series_to_supervised(data, n_in=1, n_out=1, dropnan=True):
    dff = pd.DataFrame(data)
    n_vars = dff.shape[1]
    cols, names = list(), list()
    for i in range(n_in, 0, -1):
        cols.append(dff.shift(i))
        names += [('var%d(t-%d)' % (j+1, i)) for j in range(n_vars)]
    # forecast sequence (t, t+1, ... t+n)
    for i in range(0, n_out):
        cols.append(dff.shift(-i))
        if i == 0:
            names += [('var%d(t)' % (j+1)) for j in range(n_vars)]
        else:
            names += [('var%d(t+%d)' % (j+1, i)) for j in range(n_vars)]
    # put it all together
    agg = pd.concat(cols, axis=1)
    agg.columns = names
    # drop rows with NaN values
    if dropnan:
        agg.dropna(inplace=True)
    return agg, names

# CV_Lasso_NMAE define the NMAE scoring function
def nmae(y_true, y_pred):
     return np.mean(np.absolute(y_true - y_pred) )/ np.mean(y_true)
    
def mae(y_true, y_pred):
    return np.mean(np.absolute(y_true - y_pred) )

def rmse(y_true, y_pred):
    return math.sqrt(np.mean((y_true - y_pred)**2 ))


