# UDP Round Trip Measurement

## Compiling

    mkdir build && cd build
    cmake ..
    make

## Usage
### Network Master

    ./master <Slave IPv4 Address> <Slave Port Number> <Nr. of messages> <log filename>

#### Example
Connecting to slave on 
- IP address 127.0.0.1 listening on 
- port 5. Do 
- 5 round trip measurements and store results to file 
- ../log/test.csv:

```bash
    ./master 127.0.0.1 12345 5 ../log/test.csv
```

## Notes

### Git store credentials:

    git config --global credential.helper store

### Build/Debug on target:
    
    sudo apt install cmake

#### install VScode plugins:

- CMake
- CMake Tools
- C/C++
- C/C++ Extension Pack

#### Static IP on RPi:

https://raspberrypi-guide.github.io/networking/set-up-static-ip-address#:~:text=Get%20a%20static%20IP-address,-To%20get%20a&text=You%20can%20also%20use%20the,ip-address%2C%20e.g.%20192.168.

#### Wi-Fi headless on RPi:

https://forums.raspberrypi.com/viewtopic.php?t=360175

internet route over wifi:

    sudo ip route del 0.0.0.0/0 via 192.168.99.1

#### Stress CPU:

    sudo apt install stress
    stress --cpu <nr of cores>


