FROM ubuntu:22.04

ENV DEBIAN_FRONTEND noninteractive
ENV NPROC 16

# Get all packages
RUN apt update
RUN apt install -y git ranger nano
RUN apt install -y liburcu-dev libuv1-dev libssl-dev libnghttp2-dev libxml2-dev liblmdb-dev libjson-c-dev pkg-config autoconf automake autotools-dev libtool-bin libjemalloc-dev libedit-dev libcap-dev libidn2-dev libkrb5-dev libmaxminddb-dev zlib1g-dev python3-ply astyle cmake gcc ninja-build python3-pytest python3-pytest-xdist unzip xsltproc doxygen graphviz python3-yaml valgrind
RUN apt install -y net-tools perl python3 pip libnet-dns-perl gnutls-bin
RUN pip install -U pytest dnspython pytest-xdist sslyze requests hypothesis

######################################## CLONE

# Get openssl
WORKDIR /my
RUN git clone --depth 1 --branch openssl-3.2.0 https://github.com/openssl/openssl.git
#RUN git clone --depth 1 https://github.com/open-quantum-safe/liboqs.git
RUN git clone --depth 1 --branch 0.7.2 https://github.com/open-quantum-safe/liboqs.git
RUN git clone --depth 1 https://github.com/verisign/MTL.git

######################################## OPENSSL

# Remove the default openssl
RUN apt purge openssl -y

# Build openssl
WORKDIR /my/openssl
ENV OPENSSL_DIR="/my/myopenssl"
RUN ./Configure --prefix=/usr/local --openssldir=/usr/local/ssl && \
	make -j ${NPROC} && \
	make -j ${NPROC} install LIBDIR=lib

# ...
RUN ldconfig
RUN ln -s /usr/local/bin/openssl /usr/bin/openssl
# RUN export PATH=/usr/local/bin:$PATH
# RUN export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH
# RUN source /root/.bashrc

######################################## LIBOQS

WORKDIR /my/liboqs
RUN mkdir build && \
    cd build && \
    cmake -GNinja .. -DBUILD_SHARED_LIBS=ON && \
    ninja && \
    ninja install
RUN ldconfig

######################################## MTL

WORKDIR /my/MTL
RUN autoreconf --install && \
    ./configure && \
    make -j ${NPROC}

######################################## STARTUP

STOPSIGNAL SIGTERM
WORKDIR /my/
# Configure ranger and open a shell
ENTRYPOINT \
    # Configure ranger
    ranger --copy-config=all > /dev/null 2>&1 && \
    echo "set show_hidden true" >> /root/.config/ranger/rc.conf && \
    # Open a shell
    /bin/bash
    
######################################## USE

# docker build -t my-mtl-image .
# docker run --rm -it my-mtl-image

# cd /my/MTL/examples
# echo "aaa" > a0.msg
# ./mtltool keygen a0.key "SPHINCS+-MTL-SHAKE-128S-SIMPLE" ; echo $?
# ./mtltool sign a0.key a0.msg a0.sig ; echo $?
# ./mtltool verify a0.key a0.msg a0.sig ; echo $?

# OK WORKING