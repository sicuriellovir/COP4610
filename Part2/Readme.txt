To build the module:
--------------------
sudo apt-get install build-essential linux-headers-$(uname -r)
make

To insert the module:
---------------------
insmod my_timer.ko

To remove module:
------------------
rmmod my_timer.ko

To read data:
--------------
cat /proc/timer