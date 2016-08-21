# dService Creation

## Essential components

A dService is a docker image that meets the following:
* switches to a non-root user in the Dockerfile
* has a /drunner folder
* has a /drunner/service.lua script

A minimal example that supports both dtest and ddev is here:
[Minimal Example of service.lua](https://github.com/drunner/minimalexample/blob/master/drunner/service.lua)

A more complete example is with the minecraft dService here:
[Minecraft's Example of service.lua](https://github.com/drunner/minecraft/blob/master/drunner/service.lua)

## Configuration

## Volumes

## Helper functions
| Function         |   Description
|:----------------|:--------------|
| `drun( command, arg1, arg2, ...)` | Runs the command on the host, returns the exit code. Substitutes any configuration variables, e.g. ${SERVICENAME}. |
| `drun_output( command, arg1, arg2, ...)` | Run the command on the host, returns a trimmed version of anything in stdout as a string. |
| `dstop( containername )` | stops the given container if it's running. |


## Hooks

dRunner provides quite a few hooks for you to use in your servicerunner script. This can allow you to do things like pause your service before a backup is made. A summary of hooks:

| Hook             |  Description
|:----------------:|:--------------:|
| install_end      | |
| backup_start     | |
| backup_end       | |
| restore_start    | |
| restore_end      | |
| uninstall_start  | |
| obliterate_start | |
| servicecmd_start | |
| servicecmd_end   | |
| update_start     | |
| update_end       | |
| enter_start      | |
| enter_end        | |
| status_start     | |
| status_end       | |


## Example Workflow without dProject

* Get dRunner installed on your dev machine.
* Fork/copy an existing project (e.g. [drunner/minimalexample](https://github.com/drunner/minimalexample))
* Use ddev configure to name the project something different
* Modify something - e.g. add a command to say hello in service.lua
* Build it: ddev build
* Try it out manually, does your new command work?
* Test it: ddev test


## Example Workflow with dProject

Todo
