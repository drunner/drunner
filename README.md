# dRunner [![Build Status](https://travis-ci.org/drunner/drunner.svg?branch=master)](https://travis-ci.org/drunner/drunner)

## Status - In production use

This version is a major rewrite to be cross-platform. While key functionality works it's under active development and has had only a small amount of testing (primarily on Linux). The documentation is currently incomplete, particularly around making your own dServices.

## Overview

dRunner is a stand-alone executable and a set of conventions to make it easy to make, install,
configure and use compatible "dServices" via the command line interface. It works on Debian, Ubuntu, Windows and Mac OS X.

Aimed at Docker containers, dRunner eliminates the need to manually store and manage scripts to use them, or type in long docker run commands. It also provides a configurable way (per host) to bring up services that consist of multiple Docker containers. And it works fine if you're not using Docker for your service.

## Features

* Cross-platform - available for Linux, Windows and OS X.
* dRunner is a single executable with no dependencies.
* dRunner compatible Docker Images (dServices) are self contained - everything dRunner needs is inside.
* Simple discoverable commands make it easy to use dServices (no manual needed).
* Flexible configuration for each dService, stored in Docker Volume containers that are managed for you.
* Backup an entire dService to a single encrypted file, trivially restore on another machine.
* Destroying a service leaves the machine in a clean state, no cruft left.
* Everything in containers is run as a non-root user, drunner on the host also runs as non-root.
* Trivial to install a service multiple times with different configurations (e.g. mulitple minecraft servers).
* Ansible friendly for automation (see [Exit Codes](README.md#exit-codes) below).
* Small footprint: everything dRunner creates on the host is contained in one folder of your choice (apart from Docker Volumes).
* Includes simple HTTP(S) port forwarding based on domain name (dproxy), with automatic configuration.
* Scripts to control docker containers are in Lua.
* You can use Docker Compose files to define services.
* Some useful tools to build dServices and test them.



## Installation

[Installation Notes](docs/INSTALL.md)

### Trying it out

```
 drunner install helloworld
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

dRunner provides several flags to control the output to stdout:

| Flag    |      Mode      |  dRunner output | Support calls  | dService output |
|:-------:|:--------------:|:---------------:|:--------------:|:-------------------:|
|         | default        | info and above  |                | yes           |
| -v      | verbose        | debug and above | yes      | yes           |
| -o      | capture output | errors only     |                | yes           |
| -s      | silent         | errors only     |                |                     |

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

dRunner includes a plugin called dbackup to provide management of rolling backups. After installing dRunner try
```
dbackup help
```

## Security
See [Docker's Security Statement](https://docs.docker.com/engine/security/security) for information on security and docker.


## Creating your own dService

To see how to make a dService read the [dService documentation](docs/DSERVICE.md).



## Developing dRunner itself

If you want to compile dRunner from source, read the [dRunner development documentation](docs/DRUNNERDEV.md).
