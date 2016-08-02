#!/bin/bash

# Script to install docker on a clean Debian machine.
# Should be idempotent and suitable for use from Ansible.

# return value:
#     0 if success
#     1 if error
#     3 if nothing changed

RVAL=3

if [ $(uname -rv | grep -c "Debian") -eq 0 ]; then
  echo "This script is only for Debian hosts."
  exit 1
fi

# install package if it doesn't exist, updating cache
function checkinstall {
if [ $(dpkg-query -W -f='${Status}' ${1} 2>/dev/null | grep -c "ok installed") -eq 0 ];
then
  echo "Installing ${1}.\nUpdating apt..." >&2
  apt-get update >/dev/null 2>&1
  echo "Installing..." >&2
  apt-get install -qq ${1} >/dev/null 2>&1
  echo "Done.">&2
  RVAL=0
fi
}

# ------------ INSTALL DOCKER --------------------------

checkinstall apt-transport-https

# add the dockerproject repository
if [ ! -e /etc/apt/sources.list.d/docker.list ]; then
  apt-key adv --keyserver hkp://p80.pool.sks-keyservers.net:80 --recv-keys 58118E89F3A912897C070ADBF76221572C52609D
  echo deb https://apt.dockerproject.org/repo debian-jessie main > /etc/apt/sources.list.d/docker.list
  RVAL=0
fi

checkinstall docker-engine

# these bits are not checked for changes
systemctl enable docker > /dev/null 2>&1
systemctl start docker > /dev/null 2>&1

# also install docker-compose.
DCURL="https://github.com/docker/compose/releases/download/1.7.1/docker-compose-Linux-x86_64"
DEST=/usr/local/bin/docker-compose

checkinstall curl
curl ${DCURL} -o ${DEST} --silent --location && chmod 0755 ${DEST}

exit ${RVAL}
