FROM ubuntu:20.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get -y update && \
    apt-get -y install software-properties-common && \
    apt-add-repository -y -n 'ppa:mhier/libboost-latest' && \
    apt-get -y update && \
    apt-get -y install build-essential cmake pkg-config make ninja-build && \
    apt-get -y install clang-12 clang-tidy-12 clang-format-12 llvm-12 && \
    apt-get -y install libclang-cpp12-dev libstdc++-10-dev libboost1.74-dev && \
    apt-get -y install libyaml-cpp-dev nlohmann-json3-dev libfmt-dev libgtest-dev && \
    apt-get -y install bzip2 liblzma-dev libbz2-dev zlib1g-dev libzip-dev && \
    apt-get -y install curl bash wget valgrind && \
    apt-get -y install git && \
    apt-get -y install cc65 && \
    apt-get clean

RUN update-alternatives --install /usr/bin/cc cc /usr/bin/clang-12 100 && \
    update-alternatives --install /usr/bin/c++ c++ /usr/bin/clang++-12 100 && \
    update-alternatives --install /usr/bin/clang clang /usr/bin/clang-12 100 && \
    update-alternatives --install /usr/bin/clang++ clang++ /usr/bin/clang++-12 100 && \
    update-alternatives --install /usr/bin/clang-format clang-format /usr/bin/clang-format-12 100

# Build libzippp library
RUN git clone https://github.com/ctabin/libzippp.git && \
    cd libzippp && \
    mkdir build && \
    cd build && \
    cmake .. -DCMAKE_BUILD_TYPE=Release -DLIBZIPPP_BUILD_TESTS=OFF && \
    cmake --build . --target all install && \
    cd ../.. && \
    rm -rf libzippp

# Build GTest library
RUN cd /usr/src/googletest && \
    cmake . && \
    cmake --build . --target install

CMD [ "/bin/bash" ]
