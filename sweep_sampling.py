# -*- coding: utf-8 -*-

# This example configures the receiver for a basic sweep and
# plots the sweep. The x-axis frequency is derived from the start_freq
# and bin_size values returned after configuring the device.

from bbdevice.bb_api import *
import time
import os
import numpy as np
import click

import matplotlib.pyplot as plt
import seaborn as sns; sns.set() # styling

RBW = 10.0e3 #Resolution bandwidth in Hz, all windows use zero-padding to achieve arbitrary RBWs
VBW = 10.0e3 #Video bandwidth in Hz. VBW must be less than or equal to RBW
SWEEPTIME = 0.001 #Sweep time in seconds, in [0.001 – 0.1]. The amount of time the device will spend collecting data before processing

CENTERF = 950.0e6
SPANF = 100.0e6

@click.command()
@click.argument('trace_folder', type=str, default='test')
@click.option('-n', '--samples', type=int, default=10)
@click.option('-i', '--interval', type=float, default=0.01)

def main(samples, trace_folder, interval):
    # Open device
    handle = bb_open_device()["handle"]

    # Configure device
    # spectrum frequency configuration, CENTERF, SPANF
    bb_configure_center_span(handle, CENTERF, SPANF)
    bb_configure_ref_level(handle, -30.0)
    bb_configure_gain_atten(handle, BB_AUTO_GAIN, BB_AUTO_ATTEN)
    # sweeping resolution configuration
    bb_configure_sweep_coupling(handle, RBW, VBW, SWEEPTIME, BB_RBW_SHAPE_FLATTOP, BB_NO_SPUR_REJECT)
    bb_configure_acquisition(handle, BB_MIN_AND_MAX, BB_LOG_SCALE)
    bb_configure_proc_units(handle, BB_POWER)

    # Initialize
    bb_initiate(handle, BB_SWEEPING, 0)
    query = bb_query_trace_info(handle)
    trace_len = query["trace_len"]
    start = query["start"]
    bin_size = query["bin_size"]

    # Create a folder for the collection
    trace_folder = '.\\' + trace_folder + '\\'
    if not os.path.isdir(trace_folder):
        os.mkdir(trace_folder)
    else:
        print("The folder is existed!")

    # Get sweep
    for x in range(0,samples):
        print("Loop" + str(x))
        d = bb_fetch_trace_32f(handle, trace_len)["trace_max"]
        freqs = [start + i * bin_size for i in range(trace_len)]
        time.sleep(interval)
        np.savetxt(trace_folder + "data" + str(x) + ".csv", list(zip(freqs, d)), delimiter=',')


    # Device no longer needed, close it
    bb_close_device(handle)

    # Plot
    freqs = [start + i * bin_size for i in range(trace_len)]
    plt.plot(freqs, d)
    plt.show()

if __name__ == "__main__":
    main()
