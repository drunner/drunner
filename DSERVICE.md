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
