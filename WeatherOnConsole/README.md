# WeatherOnConsole (woc) v 0.4

## Synopsis

woc is a terminal application that displays the weather forecast of 807 Canadian towns and cities with data provided by Environment Canada. 

## Screenshot

![alt text](img/screenshot.jpg "woc")

## Options
 
woc \[city name\]

woc -l \(list cities\)

woc -d \[city name\] \(set default\)

## Files

/etc/woc/wocdb - System-wide city database file. Run woc-util.sh to generate an up-to-date list. 		

~/.wocdef - Per-user default city config file created after running woc -d \[city name\].

/usr/bin/woc - Location of installed program. 

## Requirements

**Ubuntu Linux 15+:**

libxml2, libxml2-dev, libcurl3 and libcurl3-dev are required.

To compile:  
$ sudo make install

This will install woc to /usr/bin/woc, create /etc/woc/wocdb, and install man page.

**OpenBSD 5.9:**

Ports Collection must be installed first. libxml2 and curl are required.

cd /usr/ports/textproc/libxml

make install

cd /usr/ports/net/curl

make install

Edit: /usr/local/include/libxml2/encoding.h

Update line including iconv.h to:

\#include \</usr/local/include/iconv.h\>  

To compile:

$ gcc woc.c -o woc -L/usr/local/lib -lcurl -I/usr/local/include/ -lxml2 -lm -I/usr/local/include/libxml2

## Examples

$ ./woc -l | grep otta

ottawa (kanata - orleans)

ottawa (richmond - metcalfe) 

$ ./woc -d "ottawa (kanata - orleans)"
 
## License

Copyright (c) Matthew Wilson, 2015-2017.
License GPLv3+: GNU GPL version 3 or later http://gnu.org/licenses/gpl.html.
This is free software: you are free to change and redistribute it.
There is NO WARRANTY, to the extent permitted by law.
