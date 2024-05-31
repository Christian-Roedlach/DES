# Requirements

- library interrupts
    - https://github.com/phil-lavin/raspberry-pi-gpio-interrupt/blob/master/gpio-interrupt.c
    - https://github.com/WiringPi/WiringPi/tree/master
- UDP multicast
    - https://openbook.rheinwerk-verlag.de/linux_unix_programmierung/Kap11-018.htm
- adaptive struct - checked
- thread GPIO - checked
- thread timer - checked
- thread logging - checked
- thread receive - checked
- error handling - open
- test_master - open
- test on RPi (pin interrupt) - open
- services (systemd/systemctl) - open

## notes
- check thread/process priorities - checked

## syslog
- https://www.gnu.org/software/libc/manual/html_node/Syslog-Example.html

### show log:

    more /var/log/syslog | grep -a 'slave_node'

## multicast
private address range: 224.0.0.0 to 224.0.0.255
https://www.iana.org/assignments/multicast-addresses/multicast-addresses.xhtml#:~:text=The%20multicast%20addresses%20are%20in,Address%20assignments%20are%20listed%20below.
