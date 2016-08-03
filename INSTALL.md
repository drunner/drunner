# Installation Notes

## Linux

### Dependencies

dRunner needs docker. You can install it with:
```
wget -nv http://drunner.s3.amazonaws.com/lin/install_docker.sh
sudo bash install_docker.sh
```

Then give yourself permissions to run docker with:
```
sudo adduser ${USER} docker
```

### Installing dRunner
```
wget http://drunner.s3.amazonaws.com/lin/drunner
chmod a+x drunner
sudo mv drunner /usr/local/bin
drunner setup
```

If this is the first time you've installed dRunner, log out then in again to update your profile (dRunner adds its bin directory to your path in ~/.profile).


## Windows



## OS X
