#!/bin/bash -eux

# FROM: https://aur.archlinux.org/cgit/aur.git/tree/PKGBUILD?h=engine-sim-git

srcdir="$(pwd)"
pkgdir=""

sudo apt-get install -y \
    cmake \
    flex \
    g++ \
    git \
    libavcodec-dev \
    libavdevice-dev \
    libavformat-dev \
    libsdl2-dev \
    libsdl2-image-dev \
    libbison-dev \
    libboost-dev

git clone -b sdl-build https://github.com/bobsayshilol/engine-sim.git
git clone https://github.com/ange-yaghi/csv-io.git
git clone -b sdl-build https://github.com/bobsayshilol/delta-studio.git
git clone https://github.com/ange-yaghi/direct-to-video.git
git clone -b sdl-build https://github.com/bobsayshilol/piranha.git
git clone -b gcc-fixes https://github.com/bobsayshilol/simple-2d-constraint-solver.git

cd engine-sim

git submodule init
for SUBMODULE in 'csv-io' 'delta-studio' 'direct-to-video' 'piranha' 'simple-2d-constraint-solver'; do
    git submodule set-url "dependencies/submodules/${SUBMODULE}" "${srcdir}/${SUBMODULE}";
done

git submodule set-branch --branch sdl-build 'dependencies/submodules/piranha'
git submodule set-branch --branch sdl-build 'dependencies/submodules/delta-studio'
git submodule set-branch --branch gcc-fixes 'dependencies/submodules/simple-2d-constraint-solver'
git -c protocol.file.allow=always submodule update

mkdir -p build
cd build
cmake -DCMAKE_BUILD_TYPE=Release \
    -DENGINE_SIM_DATA_ROOT=/usr/share/engine-sim \
    -DDTV=OFF ..
cmake --build . -j $(nproc) --target engine-sim-app

cd ..
install -Dm644 LICENSE "${pkgdir}/usr/share/licenses/wava/LICENSE"
install -Dm755 build/engine-sim-app "${pkgdir}/usr/bin/engine-sim"
mkdir -p "${pkgdir}/usr/share/engine-sim/"
cp -r assets es "${pkgdir}/usr/share/engine-sim/"
mkdir -p "${pkgdir}/usr/share/engine-sim/dependencies/submodules/delta-studio/engines/basic/"
cp -r \
    dependencies/submodules/delta-studio/engines/basic/fonts \
    dependencies/submodules/delta-studio/engines/basic/shaders \
    "${pkgdir}/usr/share/engine-sim/dependencies/submodules/delta-studio/engines/basic/"
