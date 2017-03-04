#!/bin/bash

# Install docker on Ubuntu Trusty.
# This script is run as root (via sudo).

apt-get purge -qq lxc-docker || echo "lxc-docker not installed"
apt-key adv --keyserver hkp://p80.pool.sks-keyservers.net:80 --recv-keys 58118E89F3A912897C070ADBF76221572C52609D
echo "deb https://apt.dockerproject.org/repo ubuntu-trusty main" | tee -a /etc/apt/sources.list.d/docker.list
apt-get -qq update
apt-get install -qq libpoco-dev apt-transport-https ca-certificates linux-image-extra-$(uname -r)
apt-get install -qq apparmor
apt-get -o Dpkg::Options::="--force-confnew" install -qq docker-engine
#
wget https://github.com/docker/compose/releases/download/1.7.1/docker-compose-Linux-x86_64
sudo mv docker-compose* /usr/local/bin/docker-compose
sudo chmod a+x /usr/local/bin/docker-compose
