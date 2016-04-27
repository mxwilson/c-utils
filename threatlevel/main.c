/* threatlevel 0.1 */

/*
License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>
No warranty. Software provided as is.
Copyright Matthew Wilson, 2016.
*/

/*
to compile: make install OR:
gcc -o tl -I/usr/include/libxml2 main.c curl.c imggen.c -lxml2 -lm -lcurl

libxml2, libxml2-dev, libcurl3, libcurl3-dev and ImageMagick are required.
*/

#include<stdio.h>
#include<getopt.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<libxml/parser.h>
#include<libxml/tree.h>
#include<libxml/xmlmemory.h>
#include<time.h>
#include"tlheader.h" // includes colors and prototypes

int i=0, d=0, c=0; // command line options
int threatlevel=0;
int leftover=0;

char* savefile = "./thefile.xml"; // downloaded xml file to parse
char* urltopass = "http://rss.naad-adna.pelmorex.com/"; // source of alerts
char* imgsavedir = "./images/"; // location of saved images
char* version = "0.1";
int x=0, y=0, z=0; // count of each item read from xml file
char* theitems[5000]; // the items read
char* theitems2[5000];
char* theitems3[5000];
int decider();

// parse and grab the exact tag you want from xml file

void parseStory (xmlDocPtr doc, xmlNodePtr cur) {
xmlChar* key;
xmlChar* key2;
cur = cur->xmlChildrenNode;

while (cur != NULL) {
	if ((!xmlStrcmp(cur->name, (const xmlChar *)"title"))) {
		key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
		theitems[x] = malloc(strlen(key) + 1);
		strcpy(theitems[x], key);
		x++;
		xmlFree(key);
	}

	if ((!xmlStrcmp(cur->name, (const xmlChar *)"summary"))) {
		key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
		theitems2[y] = malloc(strlen(key) + 1);
		strcpy(theitems2[y], key);
		y++;
		xmlFree(key);
	
	}
	
	// now get the attributes - specifically msgType - <category term="xx">
	if ((!xmlStrcmp(cur->name, (const xmlChar *)"category"))) {
		key2 = xmlGetProp(cur, "term"); // xmlgetprop is key to getting attributes
		theitems3[z] = malloc(strlen(key2) + 1);
		strcpy(theitems3[z], key2);
	    	z++;
		xmlFree(key2);
	}
	
	cur = cur->next;
}

}

// parsedoc function calls parse story 

void parseDoc() {
xmlDocPtr doc;
xmlNodePtr cur;

doc = xmlParseFile(savefile);

if (doc == NULL) {
	printf("parse file error\nlikely run 'tl -d' to download new xml file\n"); 
	exit(1);
}

cur = xmlDocGetRootElement(doc);

if (cur == NULL) {
	printf("empty document\n");
	exit(1);
}

// first check if root element of doc is correct

if (xmlStrcmp(cur->name, (const xmlChar *)"feed")) {
	printf("doc of wrong type\n");
	xmlFreeDoc(doc);
	exit(1);
}

// then pick out each meta element you want, and parse

cur = cur->xmlChildrenNode;


while (cur != NULL) {
	if ((!xmlStrcmp(cur->name, (const xmlChar *)"entry"))) {
		parseStory(doc, cur);
	}

cur = cur->next;

}

xmlFreeDoc(doc);

// then call the decider to determine threat level
decider();
}

// just print the items

int actual_printer(int a, int cnt, int newcnt) {

if (newcnt != 0) {
	printf("\n");
}

if (threatlevel == 1) 	
	printf(ANSI_RED "Threat level: %d\n" ANSI_RESET, threatlevel);
else if (threatlevel == 2)
	printf(ANSI_YELLOW "Threat level: %d\n" ANSI_RESET, threatlevel);
else if (threatlevel == 3)
	printf(ANSI_GREEN "Threat level: %d\n" ANSI_RESET, threatlevel);
else if (threatlevel == 4)
	printf(ANSI_BLUE "Threat level: %d\n" ANSI_RESET, threatlevel);

printf("%d: item %d: %s\n%s\n", newcnt, a, theitems[a], theitems2[a]);
}

// decide the threatlevel of each item

int decider() {
int a;
int bb=0;
int cnt=0;
int newcnt=0;
	
// inner loop to get at 9 attributes of the alert and determine threat level

/* 1 - red - actual0, alert1, update1, cancel1, extreme5, severe5, observed7, likely7, possible7
2 - yellow-  actual0, alert1, update1, cancel1, moderate5
3 - green - actual0, exercise0, minor5
4 - blue - exercise0, system0, test0 */

for (a = 0; a < x; a++) {
	for (bb = 0; bb < 9; bb++) {
		if (strstr(theitems3[cnt], "Actual") != NULL) { 
			if  ( (strstr(theitems3[cnt+1], "Alert") != NULL) || (strstr(theitems3[cnt+1], "Update") != NULL) ||
				(strstr(theitems3[cnt+1], "Cancel") != NULL) ) { 
				if ( (strstr(theitems3[cnt+5], "Extreme") != NULL) || (strstr(theitems3[cnt+5], "Severe") != NULL) ) {
					// call the printer or image maker
					threatlevel = 1;
					actual_printer(a, cnt, newcnt); 
					if (i == 1) {
						imgmaker(a);
					}
				newcnt++; // the count of matching items
				}
			} 
		}		
	
		if (strstr(theitems3[cnt], "Actual") != NULL) { 
			if  ( (strstr(theitems3[cnt+1], "Alert") != NULL) || (strstr(theitems3[cnt+1], "Update") != NULL) ||
				(strstr(theitems3[cnt+1], "Cancel") != NULL) ) { 
				if (strstr(theitems3[cnt+5], "Moderate") != NULL) {
					threatlevel = 2;
					actual_printer(a, cnt, newcnt); 
					if (i == 1) {
						imgmaker(a);
					}
					newcnt++; 
				}
			}	 
		}
	
		if ( (strstr(theitems3[cnt], "Actual") != NULL) || (strstr(theitems3[cnt], "Exercise") != NULL) ) { 
				if (strstr(theitems3[cnt+5], "Minor") != NULL) {
					threatlevel = 3;
					actual_printer(a, cnt, newcnt); 
					if (i == 1) {
						imgmaker(a);
					}
					newcnt++; 
				}
		} 
	
		if ( (strstr(theitems3[cnt], "Exercise") != NULL) || (strstr(theitems3[cnt], "System") != NULL) ||
			(strstr(theitems3[cnt], "Test") != NULL) ) { 
			threatlevel = 4;
			actual_printer(a, cnt, newcnt); 
			if (i == 1) {
				imgmaker(a);
			}
			newcnt++; 
		}
	cnt++;		
	}
}	

printf("x:%d y:%d z:%d matched:%d\n", x, y, z, newcnt);
exit(0);
}

int thefunc(int argcc, char*argvv[]) {

// likely anything before parsing will go here

parseDoc();
}

// custom image function

int customimg() {
char title[50];
char summary[570];
int a=0;
int ln, ln2;

printf("Title: ");
fgets(title, 50, stdin);
ln = strlen(title) - 1;
if (title[ln] == '\n') {
	title[ln] = '\0';
}

printf("Summary: ");
fgets(summary, 570, stdin);
ln2 = strlen(summary) - 1;
if (summary[ln2] == '\n') {
	summary[ln2] = '\0';
}

do {
	printf("Threat level (1-4): ");
	if (fscanf(stdin, "%d", &threatlevel) == 1) {
		if (threatlevel <= 4 && threatlevel >= 1) 	
			break;
		}
		else {
			while ((threatlevel=getchar()) != '\n' && threatlevel != EOF);
		}
} while (1);

theitems[a] = malloc(strlen(title) + 1);
theitems2[a] = malloc(strlen(summary) + 1);
strcpy(theitems[a], title);
strcpy(theitems2[a], summary);

imgmaker(a); // pass it all to image maker

exit(0);
}

static int flag_help;

static struct option const long_options[] = 
{
    	{"imagegenerate", no_argument, 0, 'i'}, 
	{"help", no_argument, &flag_help, 1},
	{"custom", no_argument, 0, 'c'},
	{"download", no_argument, 0, 'd'}, 
   	{0, 0, 0, 0}			
};

void usage() {
printf("usage:\ntl (list alert items)\ntl -i (generate alert images)\ntl -d (download new xml file)\ntl -c (create custom image)\n");
exit(1);
}

int main (int argc, char* argv[]) {
int optc;
int index;

// test for imagemagick before starting

if ((access("/usr/bin/convert", F_OK) == -1)) {
	printf("imagemagick not installed\n");
	exit(1);
}

while ((optc = getopt_long (argc, argv, "idc", long_options, (int *) 0)) != EOF) {
	switch (optc) {
		case 0:
		  break; 
		case 'i':
	          i=1;
		  break;
		case 'd':
		  d=1;
		  break;
		case 'c':
		  c=1;
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
	printf("threatlevel! version: %s\n", version);
	usage();
}

if (argc == 1) {
	thefunc(argc, argv);
	
}

else if (argc > 1) {
	for (index = optind; index < argc; index++) {
		;
	}

	argc -= optind;
	argv += optind;
	leftover=index-optind;

	if ((leftover == 0) && (i == 1)) {
		thefunc(argc, argv);
	}
	if ((leftover == 0) && (c == 1)) {
		customimg();
	}
	if ((leftover == 0) && (i == 0) && (d == 1)) {
		curl_func();
		exit(0);	
	}
	else {
		usage();
	}
}

}
