
# Default root CA signature algorithm; can be set to any listed at https://github.com/rtjk/oqs-provider#algorithms
ARG SIG_ALG_CA="dilithium3"
ARG SIG_ALG_SERVER="dilithium3"

# define the Curl version to be baked in                        <-<-<-
ARG CURL_VERSION=7.81.0
#ARG CURL_VERSION=7.88.1

# define the liboqs tag to be used
ARG LIBOQS_TAG=main
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

FROM alpine:3.11

ARG SIG_ALG_CA
ARG SIG_ALG_SERVER

ARG LIBOQS_TAG
ARG OQSPROVIDER_TAG
ARG CURL_VERSION
ARG INSTALLDIR
ARG LIBOQS_BUILD_DEFINES
ARG DEFAULT_GROUPS
ARG MAKE_DEFINES

LABEL version="4"

ENV DEBIAN_FRONTEND noninteractive

RUN apk update && apk upgrade

# Get all software packages required for builing all components:
RUN apk add build-base linux-headers libtool automake autoconf cmake ninja make openssl openssl-dev git wget

# get all sources
WORKDIR /opt
RUN git clone --depth 1 --branch ${LIBOQS_TAG} https://github.com/rtjk/liboqs && \
    git clone --depth 1 --branch master https://github.com/openssl/openssl.git && \
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

WORKDIR /opt/oqssa/bin

RUN set -x && mkdir /opt/test

# generate new server CSR using pre-set CA.key & cert
RUN openssl req -new -newkey ${SIG_ALG_SERVER} -keyout /opt/test/server.key -out /opt/test/server.csr -nodes -subj "/CN=localhost" -config ${OPENSSL_CNF}

# generate server cert
RUN openssl x509 -req -in /opt/test/server.csr -out /opt/test/server.crt -CA CA.crt -CAkey CA.key -CAcreateserial -days 365

STOPSIGNAL SIGTERM
