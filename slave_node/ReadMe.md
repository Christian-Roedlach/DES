# FH Technikum Wien - DES SS2024 - Slave Node

## Authors:

Group B - Team 2:

- Christian Roedlach
- Christoph Vock

## Settings:

IP-Address:
192.168.123.2/24

Multicast address:
224.0.0.1:12345

Microtick:
500 us

Macrotick:
5 ms

GPIO-Pin:
GPIO27 PIN13


## Prerequisites

- cmake (minimum version: 3.1)
- WiringPi (https://github.com/WiringPi/WiringPi/tree/master)
- Raspberry Pi
- Signal connected to GPIO27 PIN13
- git

## Clone repository

    git clone https://github.com/Christian-Roedlach/DES.git

## Compilation

    cd DES/slave_node && mkdir build && cd build
    cmake ..
    make

## Deployment

    sudo mkdir /usr/local/bin
    sudo cp ../systemd/slave_node_wrapper.sh /usr/local/bin/
    sudo cp slave_node /usr/local/bin/
    sudo chmod +x  /usr/local/bin/slave_node /usr/local/bin/slave_node_wrapper.sh
    sudo cp ../systemd/slave_node.service /etc/systemd/system/

## Systemd

    # Register slave_node.service
    sudo systemctl daemon-reload

    # Enable (start at bootup)
    sudo systemctl enable slave_node.service

    # Start
    sudo systemctl start slave_node.service

    # Stop
    sudo systemctl stop slave_node.service

    # Disable (don't start at bootup)
    sudo systemctl disable slave_node.service

## Logfiles

The logfiles are stored to directory /var/log/slave_node

    /var/log/slave_node/timestamps_2.csv
    /var/log/slave_node/exit_state

## Syslog

The program logs are written to /var/log/syslog.

    # install on RPi bookworm
    sudo apt install rsyslog
    
    cat /var/log/syslog | grep -a 'slave_node'

## RPi static IP
(https://raspberrypi-guide.github.io/networking/set-up-static-ip-address#:~:text=Get%20a%20static%20IP-address,-To%20get%20a&text=You%20can%20also%20use%20the,ip-address%2C%20e.g.%20192.168)

1. type 'sudo nmtui' so you have the right permissions
2. edit the connection you want
4. change ipv4 config to manual
5. Enter your desired ip address into addresses (with a trailing /24, e.g. 192.168.1.77/24)
6. I put my routers ip in the gateway and DNS fields, and also added a second 8.8.8.8 for DNS
7. Exit out of nmtui
8. reboot and it should work

