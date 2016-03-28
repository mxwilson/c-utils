# WeatherOnConsole (woc) v 0.2

## Synopsis

woc is a terminal application that displays the weather forecast of many Canadian cities with data provided by Environment Canada. 

## Options
 
woc \[city name\]

woc -l \(list cities\)

woc -d \[city name\] \(set default\)

## Cities in db
Victoria, Vancouver, Kelowna, Kamloops, Prince Rupert, Edmondton, Calgary, Lethbridge, Uranium City, Saskatoon, Regina, Flin Flon, Brandon, Winnipeg, Thunder Bay, Sudbury, Ottawa, Toronto, Montreal, Sherbrooke, Quebec City, Fredericton, Moncton, Saint John, Charlottetown, Halifax, Sydney, Yarmouth, St. John's, Labrador City, Whitehorse, Yellowknife, Iqaluit.

## Requirements

libxml2, libxml2-dev, libcurl3 and libcurl3-dev are required.

To compile:  
gcc woc.c -o woc -lcurl -lxml2 -lm -I/usr/include/libxml2

## Screenshot

![alt text](img/screenshot.jpg "woc")

## License

Copyright 2015-16, Matthew Wilson. 
License GPLv3+: GNU GPL version 3 or later http://gnu.org/licenses/gpl.html.
No warranty. Software provided as is.
