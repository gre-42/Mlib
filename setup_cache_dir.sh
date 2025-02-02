#!/bin/bash

if [[ $# -ne 1 ]]; then
    echo "Usage: $(basename "$0") cache_dir" >&2
    exit 1
fi

set -eux

mkdir -p $1/Calibration
mkdir -p $1/Input
mkdir -p $1/Cameras
mkdir -p $1/TracedParticles
mkdir -p $1/OpticalFlow
mkdir -p $1/SparseReconstruction/0-2
mkdir -p $1/SparseReconstruction/0-1
mkdir -p $1/DtamReconstruction
