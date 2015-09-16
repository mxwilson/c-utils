/*
WeatherOnConsole v 0.1 
woc is an applicaton that displays current and upcoming weather conditions with data provided by Environment Canada from weather.gc.ca. Change the variable below to select the city of your choice.   

libxml2, libxml2-devel, libcurl and libcurl-devel are required to compile.

License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>
No warranty. Software provided as is.
Copyright Matthew Wilson, 2015.

TO COMPILE: 
gcc woc.c -o woc -lcurl -lxml2 -lm -I/usr/include/libxml2
*/

#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
#include<curl/curl.h>
#include<libxml/parser.h>
#include<libxml/tree.h>
#include<libxml/xmlmemory.h>

// change this variable to select your city
char* thesite = "http://weather.gc.ca/rss/city/qc-147_e.xml";

// other cities:
// http://weather.gc.ca/rss/city/bc-74_e.xml - Vancouver
// http://weather.gc.ca/rss/city/ab-52_e.xml - Calgary
// http://weather.gc.ca/rss/city/on-143_e.xml - Toronto
// http://weather.gc.ca/rss/city/qc-147_e.xml - Montreal
// http://weather.gc.ca/rss/city/ns-19_e.xml - Halifax

char* savefile = "./thefile.xml";
int x=0;
char* theitems[100];

// function for saving a downloaded page
size_t write_data(void *ptr, size_t size, size_t nmemb, FILE* stream) {
	size_t written;
	written = fwrite(ptr, size, nmemb, stream);	
	return written;
}

curl_func() {

FILE* f;
CURL* thehandle;
CURLcode res; // result of retrieval / error codes - 0 = CURLE_OK

// first initialize

curl_global_init(CURL_GLOBAL_ALL);

// and use the handle to capture upcoming transfer(s)

thehandle = curl_easy_init();

// now tell curl what to do

curl_easy_setopt(thehandle, CURLOPT_URL, thesite);

// download and save xml file

f=fopen(savefile, "wb");

if (f == NULL) {
	printf("xml savefile error :(\n");
	exit(1);
}

curl_easy_setopt(thehandle, CURLOPT_WRITEFUNCTION, &write_data);
curl_easy_setopt(thehandle, CURLOPT_WRITEDATA, f);
res = curl_easy_perform(thehandle);
fclose(f);

// now check for error

if (res != CURLE_OK) {
	remove(savefile);
	printf("curl error\n");
	exit(1);
}

// end the curl section

curl_global_cleanup();

// then call the xmlreader func

parseDoc();

}

// function called by xmlreader to zero in on each line from xml to print.
// ie - grab each line after parsing <feed>, <entry> and <title>

void parseStory (xmlDocPtr doc, xmlNodePtr cur) {
xmlChar* key;

cur = cur->xmlChildrenNode;

while (cur != NULL) {
	if ((!xmlStrcmp(cur->name, (const xmlChar *)"title"))) {
		key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
		theitems[x]=malloc(strlen(key)+1);
		strcpy(theitems[x], key);
		xmlFree(key);
	}
cur = cur->next;
}
x++;

}

// parseDoc function is the xmlreader

parseDoc() {
xmlDocPtr doc;
xmlNodePtr cur;

// point to the saved xml file

doc = xmlParseFile(savefile);

if (doc == NULL ) {
	printf("parse file error\n");
	exit(1);
}

cur = xmlDocGetRootElement(doc);

if (cur == NULL) {
	printf("empty document\n");
	exit(1);
}

// first check if root element of doc is correct
// "feed" is docroot of weather xml file

if (xmlStrcmp(cur->name, (const xmlChar *) "feed")) {
	printf("xml doc of wrong type\n");
	xmlFreeDoc(doc);
	exit(1);
}

// then pick out each sub-element you want, and zero in on each inner element
// using parseStory (above)

cur = cur->xmlChildrenNode;

while (cur != NULL) {
	if ((!xmlStrcmp(cur->name, (const xmlChar *) "entry"))) {
		parseStory(doc, cur);
	}

cur = cur->next;
}

xmlFreeDoc(doc);

int a;

// now print items or possibly do something else with them

for (a=0; a<x; a++) {
	printf("%d: %s\n", a, theitems[a]);
}

// delete xml file after use 

remove(savefile);

}

main(int argc, char* argv[]) {

if (argc > 1) {
	printf("err. plz run wituout args\n");
	exit(1);	
}
else {
	curl_func();
}

exit(0);

}

