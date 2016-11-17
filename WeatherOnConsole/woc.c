/*
WeatherOnConsole (woc) v 0.3.5
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
char* version ="0.3.5";

int default_city();
int parseDoc();
void usage();
int set_default_city(char citytopass[20]);
int curl_func(char citytopass[20], char urltopass[80]);
void parseStory (xmlDocPtr doc, xmlNodePtr cur); 
size_t write_data(void *ptr, size_t size, size_t nmemb, FILE* stream); 

int x = 0;
char* theitems[100];
int l = 0, d = 0;

// function for saving a downloaded page

size_t write_data(void *ptr, size_t size, size_t nmemb, FILE* stream) {
        size_t written;
        written = fwrite(ptr, size, nmemb, stream);
        return written;
}

// main curl function

int curl_func(char citytopass[20], char urltopass[80]) {
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
	f = fopen(savefile, "wb");

	if (f == NULL) {
        	printf("xml savefile error :(\n");
        	return(1);
	}

	curl_easy_setopt(thehandle, CURLOPT_WRITEFUNCTION, &write_data);
	curl_easy_setopt(thehandle, CURLOPT_WRITEDATA, f);
	res = curl_easy_perform(thehandle);
	fclose(f);

	// now check for error
	if (res != CURLE_OK) {
		printf("xml download fail. network connected?\n");
		return(1);
	}

	// end the curl section
	curl_global_cleanup();

	// then call the xmlreader func
	if (parseDoc() == 1) 
		return(1);
	else
		return(0);
}

// function called by xmlreader to zero in on each line from xml to print.
// ie - grab each line after parsing <feed>, <entry> and <title>

void parseStory (xmlDocPtr doc, xmlNodePtr cur) {
	xmlChar* key;

	cur = cur->xmlChildrenNode;

	while (cur != NULL) {
	       	if ((!xmlStrcmp(cur->name, (const xmlChar *) "title"))) {
                	key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
                	theitems[x] = malloc(strlen(key) + 1);
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

	if (doc == NULL) {
        	printf("xml parse file error\n");
        	return(1);
	}

	cur = xmlDocGetRootElement(doc);

	if (cur == NULL) {
        	printf("xml empty document error\n");
        	return(1);
	}

	// first check if root element of doc is correct
	// "feed" is docroot of weather xml file
	if (xmlStrcmp(cur->name, (const xmlChar *) "feed")) {
        	printf("xml doc of wrong type\n");
        	xmlFreeDoc(doc);
        	return(1);
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

	// now print the weather items
	int a;
 
	for (a = 0; a < x; a++) {
		printf("%d: %s\n", a, theitems[a]);
	}

	// delete xml file after use and end program
	remove(savefile);
	return(0);
}

// function for listing cities and matching city on command line

int tester(char argvcity[40]) {
	FILE* f;
	char line[999];
	int lncnt = 0;
	char* lineitems[1000];
	char* parsurl[1000];
	char* parscityNC[1000];
	int b;

	// check for dbfile before doing anything
	f = fopen(db_file, "r");

	if (f == NULL) {
		printf("error reading db file: %s\n", db_file);
		return(1);
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
		return(0);
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
				if (set_default_city(citytopass) != 0) {
					return(1);
				}
			}	
			// proceed to curl function to download xml file
			if (curl_func(citytopass, urltopass) == 0)
				return(0);
			else
				return(1);
		}
	}
	// otherwise no matching city/string
	printf("no match. try woc -l for city list\n");
	return(1);
}

// save default city if match is correct

int set_default_city(char citytopass[20]) {
	FILE *fp;

	if ((fp = fopen(conf_file, "w")) == NULL) {
		printf("error writing default config file: %s\n", conf_file);
		return(1);
	}	
	else {
		printf("setting default city: %s\n", citytopass);
		fprintf(fp, "%s\n", citytopass);
		fclose(fp);
	}
	return(0);
}

// no city provided on command line, attempt to load default

int default_city() {
	char argvcity[40];
	int x = 0;
	FILE *fp;
	char incomingline[150];

	// check if exist
	if (access(conf_file, F_OK) == -1) {
		printf("default city config file not exist: %s\n", conf_file);
		printf("run: woc -d [city name] to create ");
		printf("or woc -l to list\n");
		return(1);
	}
	// if exist only read top line of file. perhaps multiple lines/cities later
	if (!(fp = fopen(conf_file, "r"))) {
		printf("error reading default config file: %s\n", conf_file);
        	return(1);
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
		if (tester(argvcity) == 1)
			return(1);
	}
	return(0);
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
	if (flag_help)
		exit(EXIT_SUCCESS);
	else
		exit(EXIT_FAILURE);
}

int main (int argc, char* argv[]) {
	int optc;
	int index;
	char argvcity[40];

	while ((optc = getopt_long (argc, argv, "ld", long_options, (int *) 0)) != EOF) {
		switch (optc) {
		    case 0:
	  	    	break; 
		    case 'l':
	          	l = 1;
		  	break;
		    case 'd':
		  	d = 1;
		  	break;
	  	    case '?':
	  	  	usage();
		  	break;
	 	    default:	
	  	  	usage();
		  	break;
		}
	}

	for (index = optind; index < argc; index++) {
		;
	}

	argc -= optind;
	argv += optind;

	if (flag_help) {
		printf("weatheronconsole! version: %s\n", version);
		usage();
	}
	else if (argc == 0 && l != 1 && d != 1) {
		if (default_city() == 0)
			exit(EXIT_SUCCESS);
	}
	else if (argc == 0 && l == 1 && d != 1) {
      		if (tester(argvcity) == 0) 
			exit(EXIT_SUCCESS);
	}
	else if (argc == 1 && l != 1) {
		strcpy(argvcity, argv[0]);
		if (tester(argvcity) == 0)
			exit(EXIT_SUCCESS);
	}
	else if (argc > 1 && l != 1) {
		// concat multi-word cities
		char *final = "";
		int cc;	
		size_t thesize = 1;

		for (cc = 0; cc < argc; cc++) {
			thesize += strlen(argv[cc]) + 1;
		}
	
		malloc(thesize + 1);
		final = (char *) malloc(1 + sizeof(char*) * (thesize));
		
		// this is key for concating to prevent last space
		for (cc = 0; cc < argc; cc++) {
			strcat(final, argv[cc]);
			if (cc < (argc - 1)) {
				strcat(final, " ");
			}
		}
		// now send the string to see if match is found
		strcpy(argvcity, final);
		if (tester(argvcity) == 0)
			exit(EXIT_SUCCESS);
	}
	else {
		usage();
	}
	exit(EXIT_FAILURE);
}
