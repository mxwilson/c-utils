# WeatherOnConsole v 0.1.2 (woc)

## Synopsis

woc is an applicaton that displays current and upcoming weather conditions with data provided by Environment Canada from http://weather.gc.ca. Using the -e flag will e-mail the weather provided mailx and ssmpt are available.

## Requirements

libxml2, libxml2-devel, libcurl and libcurl-devel are required to compile.

mailx and ssmpt required for e-mail. More information here: 
https://wiki.debian.org/sSMTP

To compile:  
gcc woc.c -o woc -lcurl -lxml2 -lm -I/usr/include/libxml2

## License

Copyright 2015, Matthew Wilson. 
License GPLv3+: GNU GPL version 3 or later http://gnu.org/licenses/gpl.html.
No warranty. Software provided as is.
