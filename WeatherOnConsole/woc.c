/*
WeatherOnConsole (woc) v 0.3
Display the weather forecast of Canadian cities.

libxml2, libxml2-dev, libcurl3 and libcurl3-dev are required to compile.

TO COMPILE: 
gcc woc.c -o woc -lcurl -lxml2 -lm -I/usr/include/libxml2

License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>
No warranty. Software provided as is.
Copyright Matthew Wilson, 2015-16.
*/

#include<stdio.h>
#include<getopt.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
#include<ctype.h>
#include<curl/curl.h>
#include<libxml/parser.h>
#include<libxml/tree.h>
#include<libxml/xmlmemory.h>

char* db_file = "./.wocdb"; // db file of cities
char* conf_file = "./.wocdef"; // default city conf file
char* savefile = "./thefile.xml"; // downloaded xml file
char* version ="0.3";
int set_default_city(char citytopass[20]);
int parseDoc();
int x=0;
char* theitems[100];
int leftover=0;
int l=0, d=0;

// function for saving a downloaded page

size_t write_data(void *ptr, size_t size, size_t nmemb, FILE* stream) {
        size_t written;
        written = fwrite(ptr, size, nmemb, stream);
        return written;
}

// main curl function

void curl_func(char citytopass[20], char urltopass[80]) {
FILE* f;
CURL* thehandle;
CURLcode res; // result of retrieval / error codes - 0 = CURLE_OK

// first initialize

curl_global_init(CURL_GLOBAL_ALL);

// and use the handle to capture upcoming transfer(s)

thehandle = curl_easy_init();

// now tell curl what to do

curl_easy_setopt(thehandle, CURLOPT_URL, urltopass);

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
                theitems[x]=malloc(strlen(key) + 1);
                strcpy(theitems[x], key);
                xmlFree(key);
        }
cur = cur->next;
}
x++;
}

// parseDoc function is the xmlreader

int parseDoc() {
xmlDocPtr doc;
xmlNodePtr cur;

// point to the saved xml file

doc = xmlParseFile(savefile);

if (doc == NULL ) {
        printf("xml parse file error\n");
        exit(1);
}

cur = xmlDocGetRootElement(doc);

if (cur == NULL) {
        printf("xml empty document error\n");
        exit(1);
}

// first check if root element of doc is correct
// "feed" is docroot of weather xml file

if (xmlStrcmp(cur->name, (const xmlChar *) "feed")) {
        printf("xml doc of wrong type\n");
        xmlFreeDoc(doc);
        exit(1);
}

// then pick out each sub-element you want, and zero in on each inner element using parseStory (above)

cur = cur->xmlChildrenNode;

while (cur != NULL) {
        if ((!xmlStrcmp(cur->name, (const xmlChar *) "entry"))) {
                parseStory(doc, cur);
        }

cur = cur->next;
}

xmlFreeDoc(doc);

int a;

// now print the weather items
 
for (a = 0; a < x; a++) {
	printf("%d: %s\n", a, theitems[a]);
}

// delete xml file after use and end program

remove(savefile);
exit(0);
}

// function for listing cities and matching city on command line

int tester(char argvcity[40]) {
FILE* f;
char line[999];
int lncnt=0;
char* lineitems[1000];
char* parsurl[1000];
char* parscityNC[1000];
int b;

// check for dbfile before doing anything

f = fopen(db_file, "r");

if (f == NULL) {
	printf("error reading db file: %s\n", db_file);
	exit(1);
}

// read lines from db file. tokenize. then remove trailing space from string

while (fgets(line, sizeof(line) - 1, f) != NULL) {
	lineitems[lncnt] = malloc(strlen(line) + 1);
	strcpy(lineitems[lncnt], line);
	lineitems[lncnt][strcspn(lineitems[lncnt], "\n")] = 0;	
	
	parsurl[lncnt]=strtok(lineitems[lncnt], ",");
	parscityNC[lncnt]=strtok(NULL, ",");
	
	parscityNC[lncnt][strlen(parscityNC[lncnt]) - 1] = '\0';
	lncnt++;
}
	
fclose(f);

// list if -l flag is set

if (l == 1) {
	for (b = 0; b < lncnt; b++) {
		printf("%s\n", parscityNC[b]);
	}
	printf("\nNum of cities: %d\n", lncnt);
	exit(0);
}	

// see if there is a match

int a;
char newstring[40];

// must strip capital letters out from argv0

strcpy(newstring, argvcity);

for (a = 0; newstring[a]; a++) {
	newstring[a] = tolower(newstring[a]);
} 

// now check for matching string and proceed to curl function

int c;
char citytopass[20];
char urltopass[80];

for (c = 0; c < lncnt; c++) {
	if (strcmp(newstring, parscityNC[c]) == 0) {
		strcpy(citytopass, parscityNC[c]);
		strcpy(urltopass, parsurl[c]);
		
		// if setting default		
		if (d == 1) {
			set_default_city(citytopass);
		}	
		
		// proceed to curl function to download xml file
		curl_func(citytopass, urltopass);
	}
}

// otherwise no matching city/string

printf("no match. try woc -l for city list\n");
exit(1);
}

// save default city if match is correct

int set_default_city(char citytopass[20]) {
FILE *fp;

if ((fp = fopen(conf_file, "w")) == NULL) {
	printf("error writing default config file: %s\n", conf_file);
	exit(1);
}	

else {
	printf("setting default city: %s\n", citytopass);
	fprintf(fp, "%s\n", citytopass);
	fclose(fp);
}
}

// no city provided on command line, attempt to load default

void default_city() {
char argvcity[40];
int x=0;
FILE *fp;
char incomingline[150];

// check if exist

if (access(conf_file, F_OK) == -1) {
	printf("default city config file not exist: %s\n", conf_file);
	printf("run: woc -d [city name] to create ");
	printf("or woc -l to list\n");
	exit(1);
}

// if exist only read top line of file. perhaps multiple lines/cities later

if (! (fp = fopen(conf_file, "r")) ) {
	printf("error reading default config file: %s\n", conf_file);
        exit(1);
}

else {
	while (x != 1) { 
		if (fgets(incomingline, sizeof(incomingline) - 1, fp) != NULL) {
			strcpy(argvcity, incomingline);		
			// key to dealing with newline from file
			argvcity[strcspn(argvcity, "\n")] = 0;
			x++; // stop after reading one line
		}
	}
	fclose(fp);
	// pass the line to the tester function to check for city match
	tester(argvcity);
}

}

static int flag_help;

static struct option const long_options[] = 
{
	{"l", no_argument, 0, 'l'},
	{"d", no_argument, 0, 'd'}, 
    	{"help", no_argument, &flag_help, 1}, 
   	{0, 0, 0, 0}			
};

void usage() {
	printf("usage: woc [city name]\nwoc -l (list cities)\n");
	printf("woc -d [city name] (set default)\n");
	exit(1);
}

void main (int argc, char* argv[]) {
int optc;
int index;
char argvcity[40];

while ((optc = getopt_long (argc, argv, "ld", long_options, (int *) 0)) !=EOF) {
	switch (optc) {
		case 0:
	  	  break; 
		case 'l':
	          l=1;
		  tester(argvcity);
		  break;
		case 'd':
		  d=1;
		  break;
	  	case '?':
	  	  usage();
		  break;
	 	default:	
	  	  usage();
		  break;
	}
}

if (flag_help) {
	printf("weatheronconsole! version: %s\n", version);
	usage();
}

if (argc == 1) {
	default_city();
}

else if (argc > 1) {
	for (index = optind; index < argc; index++) {
		;
	}

	argc -= optind;
	argv += optind;
	leftover = index - optind;

	if (leftover == 0 && d == 1) {
		usage();
	}

	if (leftover == 1) {
		strcpy(argvcity, argv[0]);
		tester(argvcity);
	}

	// concat multi-word cities from command line into one string
	if (leftover > 1) {
		char *final = "";
		int cc;	
		size_t thesize = 1;

		for (cc = 0; cc < leftover; cc++) {
			thesize += strlen(argv[cc]) + 1;
		}
	
		malloc(thesize + 1);
		final = (char *) malloc(1 + sizeof(char*) * (thesize));
		
		for (cc = 0; cc < leftover; cc++) {
			strcat(final, argv[cc]);
			// this is key for concating to prevent last space
			if (cc < (leftover - 1)) {
				strcat(final, " ");
			}
		}

		// now send the string to see if match is found
		strcpy(argvcity, final);
		tester(argvcity);
	}
}

}
