#!/usr/bin/env python3

import csv
from itertools import groupby

import matplotlib.pyplot as plt
import numpy as np
from scipy.io import wavfile
from scipy.signal import stft

def remix():
    TIME0 = 0
    TIME1 = 1
    LABEL = 2

    NMIN = 10
    NPERIODS_LEFT = 1
    NPERIODS_RIGHT = 1

    with open('/tmp/engine_labels.txt', 'r') as f:
        labels = list(csv.reader(f, delimiter='\t'))
    with open('/tmp/motor.wav', 'rb') as f:
        samplerate, data = wavfile.read(f)
    if data.shape[0] == 0:
        return
    time_offset = (data.shape[0] - 1) / samplerate - float(labels[-1][TIME1])
    print(f'Time offset: {time_offset}')

    index = 0
    for key, values_iter in groupby(labels[:-1], key=lambda l: l[LABEL]):
        values = list(values_iter)
        if len(values) < NMIN:
            continue
        begin0 = int(np.round((float(values[NMIN//2 - NPERIODS_LEFT][TIME0]) + time_offset) * samplerate))
        end0 = int(np.round((float(values[NMIN//2 + NPERIODS_RIGHT][TIME0]) + time_offset) * samplerate))
        print(key, begin0, end0)
        if begin0 < 0:
            continue
        shift = (end0 - begin0) // 2
        begin1 = begin0 + shift
        end1 = end0 + shift
        if end1 > data.shape[0]:
            continue
        fade_in = np.linspace(0, 1, shift)
        fade_out = np.linspace(1, 0, shift)
        left = np.clip(
            fade_in * data[begin0:begin1] + fade_out * data[end0:end1],
            np.iinfo(np.int16).min,
            np.iinfo(np.int16).max).astype(np.int16)
        right = data[begin1:end0]
        # from IPython import embed
        # embed()
        wavfile.write(f'''/tmp/motor_{index:05}_{key.replace('.', '_')}.wav''', samplerate, np.concatenate([left, right]))
        index += 1

    if False:
        # f, t, Zxx = stft(data[(2576384-1000):2576384])
        f, t, Zxx = stft(data, nperseg=20000)

        f0 = np.argmax(np.abs(Zxx) > np.median(np.abs(Zxx), axis=0)[None, :] * 1000, axis=0)
        # f0 = np.argmax(np.abs(Zxx) > np.percentile(np.abs(Zxx), 98, axis=0)[None, :], axis=0)

        def plot_spectrum():
            plt.pcolormesh(t, f, np.abs(Zxx), shading='gouraud')
            plt.title('STFT Magnitude')
            plt.ylabel('Frequency [Hz]')
            plt.xlabel('Time [sec]')
            plt.show()

        def plot_f0():
            print(Zxx.shape)
            print(f0.shape)
            plt.plot(f0)
            plt.show()

        plot_f0()

if __name__ == '__main__':
    remix()
