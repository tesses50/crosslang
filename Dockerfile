FROM ubuntu:noble
RUN apt update -y && \
    apt install -y --no-install-recommends \
    libjansson-dev cmake git ca-certificates wget libmbedtls-dev g++ gcc libsdl2-dev libsdl2-image-dev libsdl2-ttf-dev build-essential && \
    apt clean -y && \
    rm -rf /var/lib/apt/lists/*
WORKDIR /tmp
RUN git clone https://gitea.site.tesses.net/tesses50/tesses-framework && cd tesses-framework && mkdir build && cd build && cmake -S .. -B . && make -j4   && make install && cd /tmp && rm -r /tmp/tesses-framework
COPY . /src
WORKDIR /src
RUN mkdir build && cd build && cmake -S .. -B . && make -j4 && make install && cd / && rm -r /src
WORKDIR /
