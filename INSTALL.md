# Installation Instructions

## Linux

### Dependencies

dRunner needs docker. You can install it with:
```
wget -nv http://drunner.s3.amazonaws.com/lin/docker-install
sudo bash docker-install
```

Then give yourself permissions to run docker with:
```
sudo adduser ${USER} docker
```

You'll need to log out then in again for the group to take effect.

### Installing dRunner
```
wget http://drunner.s3.amazonaws.com/lin/drunner-install
bash drunner-install
```

If this is the first time you've installed dRunner, log out then in again to update your profile (dRunner adds its bin directory to your path in ~/.profile).


## Windows
Install Docker for Windows:
```
https://docs.docker.com/docker-for-windows/
```

Download the Windows native build from:

```
https://s3-ap-southeast-2.amazonaws.com/drunner/win/drunner.exe
```
Double click it to install.

Make sure to enable sharing to your C: drive (if that's where your User folder is) in
the Docker settings. Test it works using the command shown in the Docker Shared Drives
screen.

Add `C:\Users\[USERNAME]\.drunner\bin` to your path in Windows.

Open powershell, now you should be able to run drunner.


## OS X
