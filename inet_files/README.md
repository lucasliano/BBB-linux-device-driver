# Internt connection forwarding

The purpose of this document is to explain how to setup the beaglebone-black SBC internet connection.

We will configre some rules to grant permission to the host computer to forward IP packets to the USB interface.

The first step will be to copy the `interfaces` and `resolv.conf` files in the BBB board. We can do this in the SD card or over an SSH connection. Here we will use the SSH:

- `sudo scp interfaces debian@192.168.7.2:/etc/network/interfaces`
- `sudo scp resolv.conf debian@192.168.7.2:/etc/resolv.conf`


Then we will copy the internet_bbb script to the beaglebone home directory:

- `scp internet_bbb.sh debian@192.168.7.2:/home/debian/`


# Running scripts

You'll need to run both internet scripts every time you reboot any device. Both scripts need permision:


## Host
Running this script enables forwarding:

```
lucas@hostname:~$ sudo ./internet_host.sh
[sudo] password for lucas:
net.ipv4.ip_forward = 1
-A FORWARD -i enx04a316e8f797 -o wlo1 -j ACCEPT
-A FORWARD -i wlo1 -o enx04a316e8f797 -j ACCEPT
```

Keep in mind that you will need to configure the input and output interfaces in the script. You can check those by running `ifconfig`.


## BBB
Running this script sets the default gateway config:

```
debian@BeagleBone:~$ sudo ./internet_bbb.sh
```