#!/bin/bash

sudo mknod /dev/mydevicefile1 c $1 0 	&& \
sudo chmod 660 /dev/mydevicefile1 	&& \
sudo chown :mydev1grp /dev/mydevicefile1
