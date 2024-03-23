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

