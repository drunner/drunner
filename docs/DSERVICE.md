# dService Documentation

This documentation outlines how to create a dService.

## Essential components

A dService at minimum is a service.lua file in a git repository.

A minimal example is here:
[Minimal Example of service.lua](https://github.com/drunner/minimalexample/blob/master/service.lua)

A full-on example is with the rocket.chat dService here:
[Rocket.chat example of service.lua](https://github.com/j842/rocketchat/blob/master/drunner10/service.lua)

## Configuration

Call the following functions as needed near the start of your service.lua file.

```
addconfig( name, default val, description )
```

## Helper functions

dRunner gives you some nice helper functions to make life easy. Note that the entire lua file has variable substition for the pattern
${variablename} as a pre-processing step.

| Function         |  Description
|:-----------------|:---------------|
| `b = addconfig( name, default val, description )` | Defines a configuration variable. Returns true on success. |
| `s = getconfig( name )` |  Returns the value of the configuration variable. |
| `b = setconfig( name, value )` | ets the value of a configuration variable. Returns true on success. |
| `b,s = drun( command, arg1, arg2, ...)` |  Runs the command from drundir. Returns true on success and the output of the command (trimmed).|
| `s = dsub( string )` | Returns the string with variables substituted. See note above on pre-processing substitution. |
| `s = dsplit( string )` |  Splits a command line string into a Lua table. |
| `s = getdrundir` | Returns the current directory for drun, docker and dockerti. |
| `b = setdrundir( dir )` | bool |  Sets the drun directory. If no argument passed resets to default (the dService folder on the host). |
| `s = getpwd` | Returns the current directory. |
| `docker` ||
| `dockerti` ||
| `dockerstop` ||
| `isdockerrunning` ||
| `dockerwait` ||
| `dockerpull` ||
| `dockercreatevolume` ||
| `dockerdeletevolume` ||
| `dockerbackup` ||
| `dockerrestore` ||
| `proxyenable` ||
| `proxydisable` ||
| `die` ||
| `dieif` ||
| `dieunless` ||


## Example Workflow without dProject

* Get dRunner installed on your dev machine.
* Fork/copy an existing project (e.g. [drunner/minimalexample](https://github.com/drunner/minimalexample))
