# dService Documentation

This documentation outlines how to create a dService.

## Essential components

A dService at minimum is a service.lua file in a git repository.

A minimal example is here:
[Minimal Example of service.lua](https://github.com/drunner/minimalexample/blob/master/service.lua)

A full-on example is with the rocket.chat dService here:
[Rocket.chat example of service.lua](https://github.com/j842/rocketchat/blob/master/drunner10/service.lua)

## Configuration

Call the following functions as needed in near the start of your service.lua file.

```
addconfig( VARIABLENAME, DEFAULTVALUE, DESCRIPTION )
```

## Helper functions

dRunner gives you some nice helper functions to make life easy. 

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


## Example Workflow without dProject

* Get dRunner installed on your dev machine.
* Fork/copy an existing project (e.g. [drunner/minimalexample](https://github.com/drunner/minimalexample))
