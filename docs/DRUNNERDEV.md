# dRunner development documentation

## Linux

Install docker, checkout drunner/drunner from github, then:
```
./build install
```

This uses a docker container to build drunner. Works fine compiling for Linux from cygwin in Windows too (via the docker beta and Cygwin's bash).


## Windows Native
* Install docker for windows ([from here](https://docs.docker.com/docker-for-windows/))
* Install conan for windows ([from here](https://github.com/conan-io/conan/releases/download/0.19.1/conan-win_0_19_1.exe))
* Checkout drunner/drunner from github
* Open conan and change to the drunner/vs directory, then run:
```
cd conandebug
conan install -s build_type=Debug -s compiler="Visual Studio" -s compiler.runtime=MTd -s arch=x86
cd ../conanrelease
conan install -s build_type=Release -s compiler="Visual Studio" -s compiler.runtime=MT -s arch=x86
```
This will create conanbuildinfo.props for debug and release builds.
* Open vs/drunner.sln, build and run (Release, x86)
* Add [your home dir]/.drunner/bin to your path (edit system environment variables in Windows control panel).
* Open powershell and you should be able run drunner commands.

## OS X
* To do
