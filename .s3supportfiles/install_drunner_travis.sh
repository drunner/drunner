#!/bin/bash

sudo apt-get -qq update
sudo apt-get install -qq apt-transport-https ca-certificates
sudo apt-key adv --keyserver hkp://p80.pool.sks-keyservers.net:80 --recv-keys 58118E89F3A912897C070ADBF76221572C52609D
echo "deb https://apt.dockerproject.org/repo ubuntu-trusty main" | sudo tee -a /etc/apt/sources.list.d/docker.list

sudo apt-get -qq update
sudo apt-get install linux-image-extra-$(uname -r)
sudo apt-get purge -qq lxc-docker
sudo apt-get install -qq apparmor
sudo apt-get -o Dpkg::Options::="--force-confnew" install -qq docker-engine

wget https://github.com/docker/compose/releases/download/1.7.1/docker-compose-Linux-x86_64
sudo mv docker-compose* /usr/local/bin/docker-compose
sudo chmod a+x /usr/local/bin/docker-compose

wget https://drunner.s3.amazonaws.com/lin/drunner
chmod a+x drunner
sudo mv drunner /usr/local/bin
drunner setup
