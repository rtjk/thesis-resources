FROM alpine:3.11 AS stage_1

ARG LIBOQS_TAG="main"

ARG LIBOQS_BUILD_DEFINES="-DOQS_DIST_BUILD=ON"
#ARG LIBOQS_BUILD_DEFINES="-DOQS_DIST_BUILD=OFF -DOQS_USE_AVX2_INSTRUCTIONS=OFF" # DISABLE AVX2

ARG INSTALLDIR="/opt/oqssa"

# CPU parallelism level when building with make
ARG MAKE_DEFINES="-j 20"

ENV DEBIAN_FRONTEND=noninteractive

# get system packages
RUN apk update && apk upgrade
RUN apk add build-base linux-headers libtool automake autoconf cmake ninja make openssl openssl-dev git wget
RUN apk add iproute2 net-tools iperf3 curl

# get sources
WORKDIR /opt
RUN git clone --depth 1 --branch ${LIBOQS_TAG} https://github.com/open-quantum-safe/liboqs

# build liboqs
WORKDIR /opt/liboqs
RUN mkdir build && cd build && cmake -G"Ninja" .. ${LIBOQS_BUILD_DEFINES} -DCMAKE_INSTALL_PREFIX=${INSTALLDIR} && ninja install


STOPSIGNAL SIGTERM
