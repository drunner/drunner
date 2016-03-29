# dService Creation

## Essential components

A dService is a docker image that meets the following:
* switches to a non-root user in the Dockerfile
* has a /drunner folder
* has a /drunner/servicerunner script/executable

Currently writing the servicerunner in bash is recommended. A minimal example is here:
[Minimal Example of servicerunner](https://github.com/drunner/minimalexample/blob/master/drunner/servicerunner)
