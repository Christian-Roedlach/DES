# Requirements

- library interrupts
    - https://github.com/phil-lavin/raspberry-pi-gpio-interrupt/blob/master/gpio-interrupt.c
    - https://github.com/WiringPi/WiringPi/tree/master
- UDP multicast
    - https://openbook.rheinwerk-verlag.de/linux_unix_programmierung/Kap11-018.htm
- adaptive struct
- thread GPIO
- thread timer
- thread logging
- thread receive
- services (systemd/systemctl)

## notes
- check thread/process priorities

## includes
#include <thread>
#include <mutex>

std::mutex transmit_data_mutex;

OR

using namespace std;
mutex transmit_data_mutex;

e.g.
std::lock_guard<std::mutex> lock(*transmit_data_mutex);

## multicast
private address range: 224.0.0.0 to 224.0.0.255
https://www.iana.org/assignments/multicast-addresses/multicast-addresses.xhtml#:~:text=The%20multicast%20addresses%20are%20in,Address%20assignments%20are%20listed%20below.
