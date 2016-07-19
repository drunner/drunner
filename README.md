# dRunner [![Build Status](https://travis-ci.org/drunner/drunner.svg?branch=master)](https://travis-ci.org/drunner/drunner)

## Status

In light production use, no known major issues.

## Overview

dRunner is a stand-alone executable and a set of conventions to make it easy to make, install,
configure and use compatible Docker containers ("dServices") on a Debian or Ubuntu host via the
command line interface.

dRunner eliminates the need to manually store and manage scripts to use the Docker containers,
or type in long docker run commands. It also provides a configurable way (per host) to bring up
services that consist of multiple Docker containers.

## Features

* dRunner compatible Docker Images (dServices) are self contained - everything dRunner needs is inside.
* Simple discoverable commands make it easy to use dServices (no manual needed).
* Flexible configuration for each dService, stored in Docker Volume containers that are managed for you.
* Backup an entire dService to a single encrypted file, trivially restore on another machine.
* Destroying a service leaves the machine in a clean state, no cruft left.
* Everything in containers is run as a non-root user, drunner on the host also runs as non-root.
* Trivial to install a service multiple times with different configurations (e.g. mulitple minecraft servers).
* Ansible friendly for automation (see [Exit Codes](https://github.com/j842/dr#exit-codes) below).
* Small footprint: everything dRunner creates on the host is contained in one folder of your choice (apart from Docker Volumes).
* Supports Docker Compose files for describing a dService's containers and basic settings.
* Some useful tools to build dServices and test them.


## Install notes

### First time installation

We assume here you have a standard user account called testuser which you'll use for drunner.

#### Dependencies

dRunner needs docker. You can install it as root with:
```
wget -nv http://drunner.s3.amazonaws.com/install_docker.sh
bash install_docker.sh
```

Then give the user you'll run dServices with (e.g. testuser) permissions to run docker with:
```
adduser testuser docker
```

### Installing dRunner

Logged in as the non-root user, download the installer and run it:
```
wget http://drunner.s3.amazonaws.com/drunner-install
chmod a+x drunner-install
./drunner-install -v ~/drunner
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

| Flag    |      Mode      |  dRunner Output | Support calls  | dService servicecmd |
|:-------:|:--------------:|:---------------:|:--------------:|:-------------------:|
|         | default        | info and above  | logged         | raw                 |
| -v      | verbose        | debug and above | logged         | raw                 |
| -l      | logged         | errors only     | logged         | logged              |
| -o      | capture output | errors only     | suppressed     | raw                 |
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

## Backups

dRunner includes a plugin called dbackup to provide management of daily/weekly/monthly backups. After installing drunner try
```
dbackup help
```

## Security
See [Docker's Security Statement](https://docs.docker.com/engine/security/security) for information on security and docker.


## Creating your own dService

To see how to make a dService [read the documentation](https://github.com/drunner/drunner/blob/master/DSERVICE.md).



## Developing dRunner itself

[Read the documentation](https://github.com/drunner/drunner/blob/master/DRUNNERDEV.md).
