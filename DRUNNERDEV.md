# Developing drunner

## Linux

Install docker, checkout drunner/drunner from github, then:
```
./build install
```

This uses a docker container to build drunner. Works fine compiling for Linux from cygwin in Windows too (via the docker beta and Cygwin's bash).


## Windows Native
* Install docker for windows ([from here](https://docs.docker.com/docker-for-windows/))
* Create a root folder (e.g. dev).
* Checkout drunner/drunner from github
* Right click on pocowin/pocowin.ps1 and select 'Run with PowerShell'
* Open vs/drunner.sln, build and run (Release, x86)
* Add [your home dir]/.drunner/bin to your path (edit system environment variables in Windows control panel).
* Open powershell and you should be able run drunner commands.

## OS X
* To do
