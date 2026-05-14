FROM nvidia/opengl:1.0-glvnd-devel-ubuntu18.04
# FROM nvidia/opengl:devel

RUN apt-get update && apt-get install -y mesa-utils

# Host
# docker build . -t glxg -f Glxgearsfile
# apt-get install nvidia-container-toolkit
# apt-get install nvidia-docker2
# systemctl restart docker
# xhost local:root or -u 1000
# docker run --rm -it -u 1000 -e NVIDIA_DRIVER_CAPABILITIES=all -e DISPLAY -v /tmp/.X11-unix:/tmp/.X11-unix --runtime nvidia glxg

# References
# https://github.com/NVIDIA/nvidia-docker
# https://github.com/NVIDIA/nvidia-docker/issues/875
# https://github.com/NVIDIA/nvidia-docker/issues/838
# https://github.com/NVIDIA/nvidia-docker/issues/1014
# https://hub.docker.com/r/nvidia/opengl
# https://hub.docker.com/r/coreyhanson/glxgears-nvidia

RUN apt-get install -y \
    libglfw3-dev \
    cmake \
    ninja-build \
    g++
