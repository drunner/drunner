# dRunner

## Status

In active development, generally working. Supports both version 1 and version 2 dServices.

## Overview

dRunner is a stand-alone executable and a set of conventions to make it easy to make, install,
configure and use compatible Docker containers ("dServices") on a Debian Jessie host via the
command line interface.

dRunner eliminates the need to manually store and manage scripts to use the Docker containers,
or type in long docker run commands. It also provides a configurable way (per host) to bring up
services that consist of multiple Docker containers.

## Features

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
* Tools to build dServices and test them.


## Install notes

### First time installation

We assume here you have a standard user account called testuser which you'll use for drunner.

#### Dependencies

dRunner needs docker. You can install it as root with:
```
wget -nv https://drunner.s3.amazonaws.com/install_docker.sh
bash install_docker.sh
```

Then give the user you'll run dServices with (e.g. testuser) permissions to run docker with:
```
adduser testuser docker
```

### Installing dRunner

Logged in as the non-root user, download the installer and run it:
```
wget https://drunner.s3.amazonaws.com/drunner-install
chmod a+x drunner-install
drunner-install -v ~/drunner
```

Log out then in again to pick up the ~/bin directory in your path, then you can run drunner. E.g. try
```
 drunner install drunner/helloworld
 helloworld run
```
helloworld is now in your path, you can run it directly, e.g. with no arguments
to see the help.

Back up helloworld to an encrypted archive (including all settings and local data),
then obliterate it, leaving the machine clean:
```
PASS=shh drunner backup helloworld hw.b
drunner obliterate helloworld
```
Restore the backup as hithere, and run it:
```   
PASS=shh drrunner restore hw.b hi
hi run
```

### Running some tests

dRunner can test containers for compatibility and functionality. Try it out with:
```
drunner install drunner/dtest
dtest test drunner/helloworld
```

### dRunner Images to play with

Other images to try:
* [minecraft](https://github.com/j842/drunner-minecraft) - really easy minecraft server.
* [simplesecrets](https://github.com/j842/drunner-simplesecrets) - store low security secrets in S3.


## General Use

Install a container (e.g. from DockerHub) that supports dr and call the service 'SERVICENAME'.
```
drunner install IMAGENAME SERVICENAME
```

Manage that service:
```
SERVICENAME COMMAND ARGS
```
The available COMMANDs depend on the service; they can be things like run and configure. You can get help on the service
which also lists the available COMMANDs with
```
SERVICENAME
```

Other commands that work on all services:
```
drunner obliterate SERVICENAME                 -- uninstalls service and removes ALL data! Leaves host in clean state.
drunner update SERVICENAME                     -- update service scripts from container (and docker pull)

PASS=? drunner backup SERVICENAME BACKUPFILE   -- backup container, configuration and local data.
PASS=? drunner restore BACKUPFILE SERVICENAME  -- restore container, configuration and local data.
```

## Flags

dRunner provides several flags to control the output mode:

| Flag    |      Mode      |  dRunner Output | dService Hooks | dService servicecmd |
|:-------:|:--------------:|:---------------:|:--------------:|:-------------------:|
| -n      | normal         | info and above  | logged         | raw                 |
| -v      | verbose        | debug and above | logged         | raw                 |
| -l      | logged         | errors only     | logged         | logged              |
| -o      | capture output | errors only     | raw            | raw                 |
| -s      | silent         | errors only     | suppressed     | suppressed          |

In addition, you can specify -d for 'development mode', which currently just skips any explicit docker pulls.

## Exit Codes

The convention for exit codes is:

| Exit Code | Description |
|:---------:|:-----------:|
| 0         | success     |
| 1         | error       |
| 3         | no change   |
| 127       | not implemented |

This is to aid Ansible use.

## Security
See [Docker's Security Statement](https://docs.docker.com/engine/security/security) for information on security and docker.


## Creating your own dService

To see how to make a dService [read the documentation](https://github.com/drunner/drunner/blob/master/DSERVICE.md).



## Developing dRunner itself

As root:
```
wget -nv -O /tmp/install_docker.sh https://goo.gl/2cxobx ; bash /tmp/install_docker.sh
apt-get install build-essential g++-multilib libboost-all-dev libyaml-cpp-dev
adduser devuser docker
```

As devuser:
```
git clone git@github.com:drunner/drunner.git
make
```
