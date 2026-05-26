#!/bin/bash

sudo rmmod waitqueue_module.ko || echo " " > /dev/null #Ensures script runs after error
make
sudo insmod waitqueue_module.ko

sudo chown :mydev1grp /sys/module/waitqueue_module/parameters/callback_uint_param
