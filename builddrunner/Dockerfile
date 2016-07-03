# Dockerfile for build environment

FROM debian
MAINTAINER j842

RUN apt-get update && \
      apt-get -y install \
      build-essential g++-multilib \
      libboost-all-dev libyaml-cpp-dev libpoco-dev \
      apt-transport-https ca-certificates wget && \
      rm -rf /var/lib/apt/lists/*

COPY ["./scripts","/scripts"]
RUN chmod a+x /scripts/*

VOLUME ["/source"]

CMD ["/bin/bash" "-c" "cd /source ; make -j4"]
