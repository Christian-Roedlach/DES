# FH Technikum Wien - DES SS2024 - Slave Node

## Authors:

- Christian Roedlach
- Christoph Vock

## Prerequisites

- cmake (minimum version: 3.1)
- WiringPi (https://github.com/WiringPi/WiringPi/tree/master)
- Raspberry Pi
- Signal connected to GPIO Pin 2

## Compilation

    cd slave_node && mkdir build && cd build
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
    /var/log/slave_node/exit_stat

## Syslog

The program logs are written to /var/log/syslog.

    cat /var/log/syslog | grep -a 'slave_node'





