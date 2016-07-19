# Developing drunner

## Linux

Install docker, checkout from github, then:
```
./build
```

This uses a docker container to build drunner. Works fine compiling for Linux from cygwin in Windows too (via the docker beta and Cygwin's bash).


## Windows Native
* Create a root folder (e.g. dev).
* Checkout drunner from github.
* Download poco into dev/poco-1.7.3 ([from here](http://pocoproject.org/releases/poco-1.7.3/poco-1.7.3.tar.gz "1.7.3"))
* start a VS command shell
```
buildwin.cmd 140 build static_md both Win32 nosamples notests
```
* Now open the drunner VS soln and build
