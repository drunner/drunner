# Installation Instructions

## Linux

### Dependencies

dRunner version 1.0 needs docker. You can install it with:
```
wget https://drunner.s3.amazonaws.com/10/lin/docker-install
sudo bash docker-install
```

Then give yourself permissions to run docker with:
```
sudo adduser ${USER} docker
```

You'll need to log out then in again for the group to take effect.

### Installing dRunner on a clean machine
```
wget https://drunner.s3.amazonaws.com/10/lin/drunner-install
bash drunner-install
```

If this is the first time you've installed dRunner, log out then in again to update your profile (dRunner adds its bin directory to your path in ~/.profile).

### Upgrading to dRunner 1.0 from version 0.9

Note down a list of current services with:
```
drunner list
```

Backup then uninstall each service one by one with 
```
drunner backup servicename
drunner uninstall servicename
```

Delete the .drunner home directory
```
rm ~/.drunner
```

Install the new version
```
wget https://drunner.s3.amazonaws.com/10/lin/drunner-install
bash drunner-install
```

Install each service, then start if necessary
```
drunner install dService servicename
servicename start
```

It should pick up the previous settings and data. 

If there are any issues you can uninstall, revert to 0.9 and install again. If all else fails you can restore from backup in 0.9.

## Windows
Install Docker for Windows:
```
https://docs.docker.com/docker-for-windows/
```

Download the Windows native build from:

```
https://s3-ap-southeast-2.amazonaws.com/drunner/10/win/drunner.exe
```
Double click it to install.

Make sure to enable sharing to your C: drive (if that's where your User folder is) in
the Docker settings. Test it works using the command shown in the Docker Shared Drives
screen.

Add `C:\Users\[USERNAME]\.drunner\bin` to your path in Windows.

Open powershell, now you should be able to run drunner.


## OS X
