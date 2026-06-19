#!/bin/bash

sudo rmmod linkedListTestModule.ko || echo " " > /dev/null #Ensures script runs after error

make
sudo insmod linkedListTestModule.ko


