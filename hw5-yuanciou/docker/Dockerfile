ARG RISCV="/risc-v"
ARG BASE_VERSION=22.04

FROM ubuntu:${BASE_VERSION} AS riscv-builder

ARG RISCV

RUN apt-get update \
    && apt-get install -y \
    autoconf automake autotools-dev curl python3 python3-pip libmpc-dev libmpfr-dev \
    libgmp-dev gawk build-essential bison flex texinfo gperf libtool patchutils bc \
    zlib1g-dev libexpat-dev ninja-build git cmake libglib2.0-dev device-tree-compiler

RUN <<EOF
    set -eux
    mkdir -p $RISCV
    cd /opt
    # NOTE: Has been moved to riscv-collab, but its URL of submodules are hardcoded.
    # Updating the link would break the submodule, so we retained the old link.
    git clone -b rvv-0.8 --recurse-submodules --shallow-submodules --depth=1 https://github.com/riscv/riscv-gnu-toolchain
    cd riscv-gnu-toolchain
    ./configure --prefix=$RISCV --with-arch=rv32gc --with-abi=ilp32d
    make -j$(nproc)
    make clean
EOF

ENV PATH="$RISCV/bin:$PATH"

RUN <<EOF
    set -eux
    cd /opt
    mkdir riscv-pk
    cd riscv-pk
    git init
    # ignore senvcfg introduced in https://github.com/riscv-software-src/riscv-pk/pull/311
    SHA=f03685954e825a974e59f278b5c8647440291fc7
    git fetch --depth=1 https://github.com/riscv-software-src/riscv-pk.git $SHA
    git reset --hard $SHA
    mkdir build && cd build
    ../configure --prefix=$RISCV --host=riscv32-unknown-elf
    make -j$(nproc)
    make -j$(nproc) install
    make clean
EOF

RUN <<EOF
    set -eux
    cd /opt
    # NOTE: Newer versions seem to poll the stdin, leading to unexpected errors.
    git clone -b v1.1.0 --depth=1 https://github.com/riscv-software-src/riscv-isa-sim.git
    cd riscv-isa-sim
    mkdir build && cd build
    ../configure --prefix=$RISCV --enable-misaligned
    make -j$(nproc)
    make -j$(nproc) install
    make clean
EOF

# final image
FROM ubuntu:${BASE_VERSION}

ARG RISCV

RUN apt-get update \
    && apt-get --no-install-recommends install -y \
    python3 python3-pip python3-setuptools python3-wheel \
    build-essential make \
    gdb \
    flex libfl-dev \
    bison libbison-dev \
    device-tree-compiler \
    dfu-util git \
    # will be installed at /usr/local/lib/python3.10/dist-packages
    && pip3 install colorama platformio \
    && rm -rf /var/lib/apt/lists/*

ENV LC_ALL=C.UTF-8
ENV LANG=C.UTF-8

# flag to prevent student to recursively enter docker environment
ENV STATUS_DOCKER_ACTIVATED DOCKER_ACTIVATED

RUN sed -i \
    -e 's@\\u@$(whoami 2>/dev/null || echo ${USER-I have no name!})@' \
    -e "s@groups@groups 2>/dev/null@" \
    /etc/bash.bashrc

COPY --link --from=riscv-builder "${RISCV}" "${RISCV}"
ENV PATH="$RISCV/bin:$PATH"
