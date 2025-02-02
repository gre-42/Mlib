#!/bin/bash -ex

dir=d1
dest_dir=/tmp/drift-$dir

mkdir -p $dest_dir

for i in {0..2600..50}; do
    URelWithDebInfo/Bin/render_depth_map_bundle --reference_time $i --packages /tmp/sfm-cache-huawei-$dir/DtamReconstruction/pkg-$i-*.json --minus_threshold 0.1 --median_filter_radius 2 --output $dest_dir/$i.png;
    # URelWithDebInfo/Bin/render_depth_map_bundle --reference_time $i --packages /tmp/sfm-cache-huawei-$dir/DtamReconstruction/pkg-$i-*.json --points /tmp/sfm-cache-huawei-$dir/SparseReconstruction/recon/recon-$i.m --minus_threshold 0.1 --median_filter_radius 2 --output $dest_dir/$i.png;
done
