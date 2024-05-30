
# define the liboqs tag to be used
ARG LIBOQS_TAG=main
#ARG LIBOQS_TAG=0.9.0
# define the oqsprovider tag to be used
ARG OQSPROVIDER_TAG=main
# Default location where all binaries wind up:
ARG INSTALLDIR=/opt/oqssa
# liboqs build type variant; maximum portability of image:
ARG LIBOQS_BUILD_DEFINES="-DOQS_DIST_BUILD=ON"
# Default KEM algorithms; can be set to any listed at https://github.com/rtjk/oqs-provider#algorithms
ARG DEFAULT_GROUPS="x25519:x448:kyber512:p256_kyber512:kyber768:p384_kyber768:kyber1024:p521_kyber1024"
# Define the degree of parallelism when building the image; leave the number away only if you know what you are doing
ARG MAKE_DEFINES="-j 4"

FROM ubuntu:20.04

ENV DEBIAN_FRONTEND noninteractive
ENV NPROC 4

ARG LIBOQS_TAG
ARG OQSPROVIDER_TAG
ARG CURL_VERSION
ARG INSTALLDIR
ARG LIBOQS_BUILD_DEFINES
ARG DEFAULT_GROUPS
ARG MAKE_DEFINES

RUN apt update

RUN apt install -y git ranger nano liburcu-dev libuv1-dev libssl-dev libnghttp2-dev libxml2-dev liblmdb-dev libjson-c-dev pkg-config autoconf automake autotools-dev libtool-bin libjemalloc-dev libedit-dev libcap-dev libidn2-dev libkrb5-dev libmaxminddb-dev zlib1g-dev python3-ply astyle cmake gcc ninja-build python3-pytest python3-pytest-xdist unzip xsltproc doxygen graphviz python3-yaml valgrind

# get all sources
WORKDIR /opt
RUN git clone --depth 1 --branch ${LIBOQS_TAG} https://github.com/open-quantum-safe/liboqs && \
    git clone --depth 1 --branch openssl-3.2 https://github.com/openssl/openssl.git && \
    git clone --depth 1 --branch ${OQSPROVIDER_TAG} https://github.com/open-quantum-safe/oqs-provider.git

# build liboqs
WORKDIR /opt/liboqs
RUN mkdir build && cd build && cmake -G"Ninja" .. ${LIBOQS_BUILD_DEFINES} -DCMAKE_INSTALL_PREFIX=${INSTALLDIR} && ninja install

# build OpenSSL3
WORKDIR /opt/openssl
RUN LDFLAGS="-Wl,-rpath -Wl,${INSTALLDIR}/lib64" ./config shared --prefix=${INSTALLDIR} && \
    make ${MAKE_DEFINES} && make install_sw install_ssldirs;

# set path to use 'new' openssl. Dyn libs have been properly linked in to match
ENV PATH="${INSTALLDIR}/bin:${PATH}"

# build & install provider (and activate by default)
WORKDIR /opt/oqs-provider
RUN ln -s ../openssl . && cmake -DOPENSSL_ROOT_DIR=${INSTALLDIR} -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=${INSTALLDIR} -S . -B _build && cmake --build _build  && cp _build/lib/oqsprovider.so ${INSTALLDIR}/lib64/ossl-modules && sed -i "s/default = default_sect/default = default_sect\noqsprovider = oqsprovider_sect/g" /opt/oqssa/ssl/openssl.cnf && sed -i "s/\[default_sect\]/\[default_sect\]\nactivate = 1\n\[oqsprovider_sect\]\nactivate = 1\n/g" /opt/oqssa/ssl/openssl.cnf && sed -i "s/providers = provider_sect/providers = provider_sect\nssl_conf = ssl_sect\n\n\[ssl_sect\]\nsystem_default = system_default_sect\n\n\[system_default_sect\]\nGroups = \$ENV\:\:DEFAULT_GROUPS\n/g" /opt/oqssa/ssl/openssl.cnf && sed -i "s/\# Use this in order to automatically load providers/\# Set default KEM groups if not set via environment variable\nKDEFAULT_GROUPS = $DEFAULT_GROUPS\n\n# Use this in order to automatically load providers/g" /opt/oqssa/ssl/openssl.cnf && sed -i "s/HOME\t\t\t= ./HOME\t\t= .\nDEFAULT_GROUPS\t= ${DEFAULT_GROUPS}/g" /opt/oqssa/ssl/openssl.cnf

# generate certificates for openssl s_server, which is what we will test curl against
ENV OPENSSL=${INSTALLDIR}/bin/openssl
ENV OPENSSL_CNF=${INSTALLDIR}/ssl/openssl.cnf

# build bind
WORKDIR /
RUN git clone --depth 1 --branch main https://github.com/Martyrshot/OQS-bind/
WORKDIR /OQS-bind
RUN autoreconf -fi
RUN ./configure --with-openssl=/opt/openssl
RUN make -j $NPROC

# >>> fips.c:37:10: error: implicit declaration of function 'FIPS_mode' [-Werror=implicit-function-declaration]

STOPSIGNAL SIGTERM

ENTRYPOINT \
	# Configure ranger
	ranger --copy-config=all > /dev/null 2>&1 && \
	echo "set show_hidden true" >> /root/.config/ranger/rc.conf && \
	# Open a shell
	/bin/bash
