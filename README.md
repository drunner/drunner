## dRunner

This is the C++ version of dRunner, inteded to soon replace http://drunner.io

# Status

In development, not yet usable.

# Overview

dRunner is a script and a set of conventions to make it easy to install,
configure and use Docker containers on a Debian host via the command line interface.

dRunner eliminates the need to separately store and manage scripts to use the Docker container,
or deal with long docker run commands.

Features:
* dRunner compatible Docker Images are self contained - everything dRunner needs is inside
* Simple discoverable commands for using compatible services (no manual needed)
* Flexible configuration for each service, stored in Docker Volume containers that are managed for you
* Services can consist of any number of containers
* Backup an entire service to a single file, trivially restore on another machine
* Destroying a service leaves the machine in a clean state, no cruft left
* Everything in containers is run as a non-root user, drunner on host runs as non-root
* Trivial to install a service multiple times with different configurations (e.g. mulitple minecraft servers)
* Ansible friendly for automation (see [Exit Codes](https://github.com/j842/dr#exit-codes) below).
* Small footprint: everything dRunner creates on the host is contained in one folder of your choice (apart from Docker Volumes).

# Dev notes

As root:
```
wget -nv -O /tmp/install_docker.sh https://goo.gl/2cxobx ; bash /tmp/install_docker.sh
apt-get install build-essential g++-multilib libboost-all-dev
adduser testuser docker
```

As testuser:
```
git clone git@github.com:j842/drunnerc.git
make
```

