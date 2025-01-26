FROM ubuntu:22.04

RUN apt-get update \
    && apt-get --no-install-recommends install -y \
    python3 python3-pip python3-setuptools python3-wheel \
    gcc \
    build-essential make \
    flex libfl-dev \
    bison libbison-dev \
    && rm -rf /var/lib/apt/lists/* \
    # will be installed at /usr/local/lib/python3.10/dist-packages
    && pip3 install --no-cache-dir colorama

# flag to prevent student to recursively enter docker environment
ENV STATUS_DOCKER_ACTIVATED DOCKER_ACTIVATED

RUN sed -i \
    -e 's@\\u@$(whoami 2>/dev/null || echo ${USER-I have no name!})@' \
    -e "s@groups@groups 2>/dev/null@" \
    /etc/bash.bashrc
