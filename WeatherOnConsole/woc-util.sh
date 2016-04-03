#!/bin/bash

#Utility for WeatherOnConsole (woc-util.sh) v 0.1

#License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>
#No warranty. Software provided as is. Copyright Matthew Wilson, 2016.

#This script will fetch the current listing of available weather feeds 
#provided by Environment Canada. It will populate the database used by
#WeatherOnConsole. 

URL="http://weather.gc.ca/rss/city/"

# delete old db file if exist

if [ -e "./.wocdb" ] 
	then 
	rm ./.wocdb
fi

mkdir xml && cd xml

# download all the files

for provinces in on qc
do
  for i in {1..180}
  do
    FILENAME="$provinces"-"$i"_e.xml
    curl -f "$URL""$FILENAME" -o "$FILENAME"
  done
done

for provinces in sk nb ns nl pe nt nu yt
do
  for i in {1..66}
  do  
    FILENAME="$provinces"-"$i"_e.xml
    curl -f "$URL""$FILENAME" -o "$FILENAME"
  done
done

for provinces in ab bc mb 
do
  for i in {1..99}
  do
    FILENAME="$provinces"-"$i"_e.xml
    curl -f "$URL""$FILENAME" -o "$FILENAME"
  done
done

# then list all of the downloaded files with title
# echo filename
# grep the page title and city from page and head 1st result
# and print city name before "- Weather" string 

for i in *.xml
do
  echo $URL$i,$(grep title $i | head -n 1) |
	awk -F '[<>]' '{print $1 $3}' | 
	awk -F '- Weather' '{print $1}' >> ../.wocdb
done

cd .. && rm -rf xml

# convert all in file to lowercase

dd if=./.wocdb of=./.wocdbLC conv=lcase &&
mv ./.wocdbLC ./.wocdb

# and remove all accented characters

iconv -f utf8 -t ascii//TRANSLIT ./.wocdb > ./.wocDBAC &&
mv ./.wocDBAC ./.wocdb

# and finally remove the space after the comma

sed -e "s/, /,/g" ./.wocdb > ./.wocsed &&
mv ./.wocsed ./.wocdb
