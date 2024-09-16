# this Dockerfile is derived from
# https://github.com/open-quantum-safe/oqs-demos/tree/main/curl
#
# 1. build liboqs, oqs-provider, openssl, and curl
# 2. create a certificate autority with SIG_ALG_CA
# 3. create a server certificate with SIG_ALG_SERVER, issued by the CA
# 4. use traffic control to simulate a network configuration (optional)
# 5. start the server
# 6. connect and measure the number of handshakes
#
# post-quantum KEMs and signature schemes available:
# https://github.com/open-quantum-safe/oqs-provider#algorithms
#
# classical KEMs and signature schemes are also available:
# openssl list -signature-algorithms
# openssl list -key-exchange-algorithms

FROM alpine:3.11 AS stage_1

# root CA signature scheme
ARG SIG_ALG_CA="dilithium3"

# KEMs
ARG DEFAULT_GROUPS="x25519:x448:kyber512:p256_kyber512:kyber768:p384_kyber768:kyber1024:p521_kyber1024"

############## software versions ##################
ARG OPENSSL_TAG="openssl-3.2"
#ARG OPENSSL_TAG=openssl-3.3 # ERROR
ARG CURL_VERSION="8.9.1"
#ARG CURL_VERSION="7.81.0"
ARG LIBOQS_TAG="add-cross"
ARG OQSPROVIDER_TAG="add-cross-allvariants"
###################################################

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
RUN git clone --depth 1 --branch ${LIBOQS_TAG} https://github.com/rtjk/liboqs && \
    git clone --depth 1 --branch ${OPENSSL_TAG} https://github.com/openssl/openssl.git && \
    git clone --depth 1 --branch ${OQSPROVIDER_TAG} https://github.com/rtjk/oqs-provider.git && \
    wget https://curl.haxx.se/download/curl-${CURL_VERSION}.tar.gz && tar -zxvf curl-${CURL_VERSION}.tar.gz;

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

WORKDIR ${INSTALLDIR}/bin
# generate CA key and cert
RUN set -x; \
    ${OPENSSL} req -x509 -new -newkey ${SIG_ALG_CA} -keyout CA.key -out CA.crt -nodes -subj "/CN=oqstest CA" -days 365 -config ${OPENSSL_CNF}

# build curl - injecting OQS CA generated above into root store
WORKDIR /opt/curl-${CURL_VERSION}

# Download and integrate LetsEncrypt Root CA to CA bundle
RUN wget https://letsencrypt.org/certs/isrgrootx1.pem -O oqs-bundle.pem && cat ${INSTALLDIR}/bin/CA.crt >> oqs-bundle.pem 

RUN env LDFLAGS=-Wl,-R${INSTALLDIR}/lib64  \
        ./configure --prefix=${INSTALLDIR} \
                    --with-ca-bundle=${INSTALLDIR}/oqs-bundle.pem \
                    --enable-debug \
                    --with-ssl=${INSTALLDIR} && \
    make ${MAKE_DEFINES} && make install && mv oqs-bundle.pem ${INSTALLDIR};

# Download current test.openquantumsafe.org test CA cert
#WORKDIR ${INSTALLDIR}
#RUN wget https://test.openquantumsafe.org/CA.crt && mv CA.crt oqs-testca.pem

################
# SECOND STAGE #
################

FROM stage_1 AS stage_2

# having a second stage saves us building time if we change only these arguments:
ARG SIG_ALG_SERVER="dilithium3"
ARG KEM="kyber512"
ARG TEST_TIME=5
ARG TC_STATUS=ON
ARG TC_DELAY=100ms
ARG TC_RATE=500mbps

# export the arguments as environmet varables to use them in ENTRYPOINT
ENV TEST_TIME=${TEST_TIME}
ENV KEM=${KEM}
ENV TC_STATUS=${TC_STATUS}
ENV TC_DELAY=${TC_DELAY}
ENV TC_RATE=${TC_RATE}

WORKDIR /opt/oqssa/bin

RUN set -x && mkdir /opt/test

# generate new server CSR using pre-set CA.key & cert
RUN openssl req -new -newkey ${SIG_ALG_SERVER} -keyout /opt/test/server.key -out /opt/test/server.csr -nodes -subj "/CN=localhost" -config ${OPENSSL_CNF}

# generate server cert
RUN openssl x509 -req -in /opt/test/server.csr -out /opt/test/server.crt -CA CA.crt -CAkey CA.key -CAcreateserial -days 365

STOPSIGNAL SIGTERM

ENTRYPOINT \
    # print cerficate size and algorithm
    wc -c /opt/oqssa/bin/CA.crt && \
    openssl x509 -in /opt/oqssa/bin/CA.crt -noout -text | grep -m 2 Algorithm && \
    wc -c /opt/test/server.crt && \
    openssl x509 -in /opt/test/server.crt -noout -text | grep -m 2 Algorithm && \
    # apply traffic control (without rate)
    [ ${TC_STATUS} != "ON" ] && echo "Debug: tc is disabled" || tc qdisc add dev lo root netem delay ${TC_DELAY} && \
    # apply traffic control (with rate)
    #[ ${TC_STATUS} != "ON" ] && echo "Debug: tc is disabled" || tc qdisc add dev lo root netem delay ${TC_DELAY} rate ${TC_RATE} && \
    # start the server
    (openssl s_server -cert /opt/test/server.crt -key /opt/test/server.key -groups ${KEM} -www -tls1_3 -accept localhost:4433 &) && \
    while ! nc -z localhost 4433 ; do sleep 0.1; done  && \
    # connect to the server
    openssl s_time -connect :4433 -new -verify 1 -time ${TEST_TIME}
