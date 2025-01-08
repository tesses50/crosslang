FROM onedev.site.tesses.net/tesses-framework/tesses-framework:latest
RUN apt update -y && \
    apt install -y --no-install-recommends \
    libjansson-dev wget libsdl2-dev libsdl2-image-dev libsdl2-ttf-dev && \
    apt clean -y && \
    rm -rf /var/lib/apt/lists/*
RUN mkdir /src && cd /src && git clone https://onedev.site.tesses.net/crosslang . && cd /src && mkdir build && cd build && cmake -S .. -B . && make -j4 && make install && cd / && rm -r /src
WORKDIR /
