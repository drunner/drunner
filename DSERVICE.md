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

Call the following functions as needed from drunner_setup() in your service.lua file.

```
function drunner_setup()
-- addconfig(NAME, DESCRIPTION, DEFAULT VALUE, TYPE, REQUIRED, USERSETTABLE)
-- addvolume(NAME, [BACKUP], [EXTERNAL])
-- addcontainer(NAME)
-- addproxy(VIRTUAL_HOST,HTTP_PORT,HTTPS_PORT)
-- addcron(offsetmin, repeatmin, function)
end
```

## Volumes

## Helper functions
| Function         |   Description
|:----------------|:--------------|
| `drun( command, arg1, arg2, ...)` | Runs command on the host, returns the exit code. Substitutes any configuration variables, e.g. ${SERVICENAME}. The command and args can also be passed as a Lua table. |
| `drun_output( command, arg1, arg2, ...)` | Same as drun, but returns a string (trimmed command output) instead of the exit code. |
| `drun_outputexit( command, arg1, arg2, ...)` | Same as drun, but returns both a string (trimmed command output) and the exit code. |
| `drunning( containername )` | Returns true if the container is currently running. |
| `dstop( containername )` | Stops the given container if it's running. |
| `dsub( string )` | returns the string where any configuration variables have been substituted. |
| `dconfig_get( key )` | returns the value of the configuration variable. |
| `dconfig_set( key, val )` | sets the value of the configuration variable. |
| `dsplit( string)` | Splits a command line string into a Lua table. |
| `getdrundir()` | Returns the current drun directory. |
| `setdrundir( dir )` | Sets the drun directory. If no argument passed resets to default (the dService folder on the host). |
| `getpwd()` | Returns the current directory. |

## Hooks

dRunner provides quite a few hooks for you to use in your servicerunner script. This can allow you to do things like pause your service before a backup is made. A summary of hooks:

| Hook             |  Description
|:----------------:|:--------------:|
| install_end      | |
| backup_start     | First argument is a temporary folder on the host that gets backed up. |
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
