# dService Creation

## Essential components

A dService is a docker image that meets the following:
* switches to a non-root user in the Dockerfile
* has a /drunner folder
* has a /drunner/servicerunner script/executable

Currently writing the servicerunner in bash is recommended. A minimal example that supports both dtest and ddev is here:
[Minimal Example of servicerunner](https://github.com/drunner/minimalexample/blob/master/drunner/servicerunner)

## Optional components

If you want to run things in a Docker container (which you usually will), you'll need to define a docker-compose.yml file
in the /drunner folder. The [minecraft](https://github.com/drunner/minecraft) example illustrates this, as well as
passing the port to use via a configured environment variable. You can see how it works by looking at the
[docker-compose.yml](https://github.com/drunner/minecraft/blob/master/drunner/docker-compose.yml) and [servicerunner](https://github.com/drunner/minecraft/blob/master/drunner/servicerunner) files.

### Volumes

dRunner can manage volumes for you, which means creating them, backing them up and restoring them automatically. To declare a dRunner managed volume you need to:
* include it in the volumes: section of docker-compose.yml,
* give it a docker-compose label that contains drunner.
* declare it as external, and
* define a name: for it.
The actual volume name can be anything, but by convention is generally of the form "drunner-${SERVICENAME}-config" etc. By including the dService name with ${SERVICENAME} we disambiguate multiple installs of the same dService.

The volume's ownership is set to the user of the corresponding service (if any). That allows you to change the ownership and permissions as you wish in the install_end hook.

Minecraft's [docker-compose.yml](https://github.com/drunner/minecraft/blob/master/drunner/docker-compose.yml) shows an example.

You can also include volumes not managed by dRunner in docker-compose.yml, whether they are host volumes, named volumes managed by docker-compose, or named volumes that are external and not managed by either docker-compose.yml or drunner for that dService (e.g. if you have a utility dService that mounts a volume from another dService, you don't want the utility or docker-compose to try and manage it).

### Environment variables

dRunner provides some environment variables:
* SERVICNAME - the local name of the installed dService as specified by the user in the drunner install command.
* IMAGENAME - the main Docker Image used for the dService.
* SERVICETEMPDIR - a directory on the host that your servicerunner can write temporary files to.
* SERVICEHOSTVOL - a directory on the host that your servicerunner can write persisting files to (included in backups).
* HOSTIP - the IP address of the host.
* DEVELOPERMODE - true if -d flag was bassed to dRunner (don't pull images).

In addition dRunner allows servicerunner to save environment variables (e.g. configuration) that are stored on the host and included in backups.

 [drunner/simplesecrets](https://github.com/drunner/simplesecrets) shows how to pass these saved environment variables through to the container using [docker-compose.yml](https://github.com/drunner/simplesecrets/blob/master/drunner/docker-compose.yml) and [servicerunner](https://github.com/drunner/simplesecrets/blob/master/drunner/servicerunner).

## Hooks

dRunner provides quite a few hooks for you to use in your servicerunner script. This can allow you to do things like pause your service before a backup is made. A summary of hooks:

| Hook             |  Extra Environment Variables |  Notes |
|:----------------:|:--------------:|:---------------:|
| install_end      | normal         | info and above  |
| backup_start     | verbose        | debug and above |
| backup_end       | logged         | errors only     |
| restore_start    | capture output | errors only     |
| restore_end      | silent         | errors only     |
| uninstall_start  | silent         | errors only     |
| obliterate_start | silent         | errors only     |
| servicecmd_start | silent         | errors only     |
| servicecmd_end   | silent         | errors only     |
| update_start     | silent         | errors only     |
| update_end       | silent         | errors only     |
| enter_start      | silent         | errors only     |
| enter_end        | silent         | errors only     |
| status_start     | silent         | errors only     |
| status_end       | silent         | errors only     |


## Example Workflow without dProject

* Get dRunner installed on your dev machine.
* Install ddev:   drunner install drunner/ddev
* Install dtest:  drunner install drunner/dtest
* Fork/copy an existing project (e.g. [drunner/minimalexample](https://github.com/drunner/minimalexample))
* Edit ddev.sh to name the project something different
* Modify something - e.g. add a command to say hello in servicerunner
* Build it: ddev build
* Try it out manually, does your new command work?
* Test it: ddev test


## Example Workflow with dProject

Todo
