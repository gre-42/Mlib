#!/usr/bin/env python3

import csv
import json
import os.path
from argparse import ArgumentParser
from itertools import groupby

import numpy as np
from scipy.io import wavfile

# Not that "MAX_DURATION" includes all periods, also the ones that
# are cropped later.
MAX_DURATION = 10
NPERIODS_MIN = 100
NPERIODS_LEFT = 80
NPERIODS_RIGHT = 10
NPERIODS_SHIFT = 2
MIN_OVERLAP_FRACTION = 10

def compute_offset(data0: np.ndarray, data1: np.ndarray) -> int:
    data0 = np.asarray(data0).astype(np.float64)
    data1 = np.asarray(data1).astype(np.float64)
    c = np.correlate(data0, data1, mode='full')
    d = np.correlate(np.ones_like(data0), np.ones_like(data1), mode='full')
    d[d < (len(data0) // MIN_OVERLAP_FRACTION)] = np.inf
    c /= d
    return np.argmax(c) - data1.shape[0] + 1

# print(compute_offset([0,1,0,0], [0,0,0,0,1,0]))

def remix(args):
    os.makedirs(args.out_dir, exist_ok=True)
    # Column-names of the labels-file.
    TIME0 = 0
    TIME1 = 1
    LABEL = 2

    with open(args.source_labels_after_engine_start, 'r') as f:
        labels = list(csv.reader(f, delimiter='\t'))
    with open(args.source_wav, 'rb') as f:
        samplerate, data = wavfile.read(f)
    if data.shape[0] == 0:
        return
    # Compute offset s.t. the beginning of the audio-file is discarded.
    time_offset = (data.shape[0] - 1) / samplerate - float(labels[-1][TIME1])
    print(f'Time offset: {time_offset}')

    meta = []
    index = 0
    for key, values_iter in groupby(labels[:-1], key=lambda l: l[LABEL]):
        times = np.array([v[TIME0] for v in values_iter], dtype=float)
        times = times[(times - times[0]) < MAX_DURATION]
        if len(times) < NPERIODS_MIN:
            continue
        def data_index(value_index):
            return int(np.round((times[value_index] + time_offset) * samplerate))
        begin0 = data_index(NPERIODS_LEFT)
        end0 = data_index(-NPERIODS_RIGHT - 1)
        end0 -= compute_offset(
            data[begin0:data_index(NPERIODS_LEFT + 1)],
            data[end0:data_index(-NPERIODS_RIGHT)])
        print(index, key, begin0, end0)
        if begin0 < 0:
            continue
        shift = (
            data_index(NPERIODS_LEFT + NPERIODS_SHIFT) -
            begin0)
        begin1 = begin0 + shift
        end1 = end0 + shift
        if end1 > data.shape[0]:
            continue
        if end1 > data_index(-1):
            raise ValueError("NPERIODS_RIGHT is too small after incorporating shift")
        fade_in = np.linspace(0, 1, shift)
        fade_out = np.linspace(1, 0, shift)
        left = np.clip(
            fade_in * data[begin0:begin1] + fade_out * data[end0:end1],
            np.iinfo(np.int16).min,
            np.iinfo(np.int16).max).astype(np.int16)
        right = data[begin1:end0]
        dest_wav_filename = f'''sample_{index:05}_{key.replace('.', '_')}.loop.wav'''
        meta.append(dict(
            filename = dest_wav_filename,
            key = key,
            frequency = (len(times) - 1) / (times[-1] - times[0]) * args.frequency_multiplier,
            # frequency = (len(times) - 1 - NPERIODS_LEFT - NPERIODS_RIGHT) / (times[-NPERIODS_RIGHT-1] - times[NPERIODS_LEFT]),
            # frequency = samplerate / (end0 - begin0 - 1) * (len(times) - 1 - NPERIODS_LEFT - NPERIODS_RIGHT)
        ))
        wavfile.write(os.path.join(args.out_dir, dest_wav_filename), samplerate, np.concatenate([left, right]))
        # wavfile.write(f'''/tmp/motor_{index:05}_{key.replace('.', '_')}.orig.wav''', samplerate, data[begin0:end1])
        # wavfile.write(f'''/tmp/motor_{index:05}_{key.replace('.', '_')}.a.wav''', samplerate, data[begin0:begin1])
        # wavfile.write(f'''/tmp/motor_{index:05}_{key.replace('.', '_')}.b.wav''', samplerate, data[end0:end1])
        index += 1
    with open(os.path.join(args.out_dir, 'meta.json'), 'w') as f:
        json.dump(meta, f, indent=2)


if __name__ == '__main__':
    parser = ArgumentParser()
    parser.add_argument("--source_labels_after_engine_start", required=True)
    parser.add_argument("--source_wav", required=True)
    parser.add_argument("--out_dir", required=True)
    parser.add_argument("--frequency_multiplier", type=float, required=True, help='Four-stroke engine: 2')

    remix(parser.parse_args())
