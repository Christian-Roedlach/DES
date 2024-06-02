#!/bin/bash
mkdir -p /var/log/slave_node
cd /var/log/slave_node

# Run the program
/usr/local/bin/slave_node 224.0.0.1 12345 timestamps_2.csv

# store return value
status=$?
echo "return value: $status"

# Handle the return value
if [ $status -eq 3 ]; then
    # Stop the service if the return value is 3
    echo "stopping service ..."
    systemctl stop slave_node.service
elif [ $status -eq 4 ] || [ $status -eq 5 ]; then
    # Stop and disable the service if the return value is 4 or 5
    echo "disabling and stopping service ..."
    systemctl stop slave_node.service
    systemctl disable slave_node.service
fi

exit $status