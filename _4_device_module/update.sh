#!/bin/bash

sudo rmmod device1_module.ko || echo " " > /dev/null #Ensures script runs after error
make
sudo insmod device1_module.ko
