FROM alpine:latest AS build

RUN apk update
RUN apk add --no-cache cmake g++ make git

COPY ./ /src


RUN cd /src && mkdir build && cd build && cmake -S .. -B . -DTESSESFRAMEWORK_ENABLE_EXAMPLES=OFF -DTESSESFRAMEWORK_ENABLE_APPS=ON -DCMAKE_BUILD_TYPE=Release ; make -j`nproc` && make install DESTDIR=/out

FROM alpine:latest
RUN apk update
RUN apk add --no-cache libstdc++
COPY --from=build /out/usr /usr

ENV CROSSLANG_CONTAINER=1
ENTRYPOINT ["/usr/local/bin/crossint"]
