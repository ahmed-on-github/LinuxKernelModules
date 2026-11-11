## Update the kernel, kernel-headers and kernel-devel tools to the same version.

### I am using yum/dnf on centos 9 stream:

$ sudo yum updategroup 'Development Tools'

$ sudo yum update kernel kernel-headers kernel-devel 

$ sudo reboot

### After reboot, ensure that u booted from the recently updpated kernel version:
$ uname -r
