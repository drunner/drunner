# Dockerfile for build environment

FROM debian
MAINTAINER j842

RUN apt-get update && \
      apt-get -y install \
      build-essential g++-multilib libboost-all-dev libyaml-cpp-dev \
      apt-transport-https ca-certificates wget && \
      rm -rf /var/lib/apt/lists/*

RUN mkdir /poco && cd /poco && export POCOVER="poco-1.7.3" && \
    wget http://pocoproject.org/releases/${POCOVER}/${POCOVER}.tar.gz && \
    gunzip ${POCOVER}.tar.gz && \
    tar xf ${POCOVER}.tar && \
    cd ${POCOVER} && \
    ./configure --static && \
    make -j4 && \
    make install && \
    cd ; rm -rf /poco

COPY ["./scripts","/scripts"]
RUN chmod a+x /scripts/*

VOLUME ["/source"]

CMD ["/scripts/build"]
