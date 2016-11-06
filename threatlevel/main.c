/* threatlevel 0.2 */

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
#include<ctype.h>
#include"tlheader.h" // includes colors and prototypes

char the_event[30]; // detects type of event received from last xml attribute ie "rainfall"
static int TL1 = 0; // used if selecting individual threatlevels to parse/print/create image 
static int TL2 = 0; // ie: $tl -s 1 3 4
static int TL3 = 0;
static int TL4 = 0;

int t = 0, i = 0, d = 0, c = 0, s = 0; // command line options
int threatlevel = 0;
int leftover = 0;

char* savefile = "./thefile.xml"; // downloaded xml file to parse
char* urltopass = "http://rss.naad-adna.pelmorex.com/"; // source of alerts
char* imgsavedir = "./images/"; // location of saved images
char* version = "0.2";
int x = 0, y = 0, z = 0; // count of each item read from xml file
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
		exit(EXIT_FAILURE);
	}

	cur = xmlDocGetRootElement(doc);

	if (cur == NULL) {
		printf("empty document\n");
		exit(EXIT_FAILURE);
	}
	
	// first check if root element of doc is correct
	if (xmlStrcmp(cur->name, (const xmlChar *)"feed")) {
		printf("doc of wrong type\n");
		xmlFreeDoc(doc);
		exit(EXIT_FAILURE);
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

	// parse the event type, get event name after stripping "event="
	char* the_event_parsed;
	int len = strlen(the_event);
	//printf("%d: %s\n", len, the_event);
	strncpy(the_event_parsed, the_event + 6, (len - 6) + 1); 
	printf("Event type: %s\n", the_event_parsed);
	printf("nc:%d item:%d\n%s\n%s\n", newcnt, a, theitems[a], theitems2[a]);

	// post to twitter feature, using the 't' program		
	if (t == 1) {
		char thebuf[400];
		int pos;
		char* ptr;
		char myarea[30];
		// find the position of area in the string
		ptr = strstr(theitems2[a], "Area:");
		pos = ptr - theitems2[a];
		
		// then copy out the substring city after "Area:"
		strncpy(myarea, theitems2[a] + pos + 6, pos + 16 - pos);
		printf("AREA: %s\n", myarea);
		snprintf(thebuf, sizeof thebuf, "t update \"#ALERT Threatlevel: %d. #%s #%s %s\"", threatlevel, the_event_parsed, myarea, theitems[a]);
		printf("BUFF: %s\n", thebuf);

		system(thebuf);
		sleep(7);	
	}
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
					threatlevel = 1; // threatlevel established, now just grab the event name "ie-rainfall"
					if (strstr(theitems3[cnt+8], "event") != NULL) {
						strcpy(the_event, theitems3[cnt+8]);
					}							
					if ( (s != 1) || ( (s == 1) && (TL1 == 1) ) ) {
						if (i != 1) { // the imagemaker also prints out a list, but doesn't have to
							actual_printer(a, cnt, newcnt); 
						}
						else {
							imgmaker(a);
						}
						newcnt++; // the count of matching items
					}
				}
			} 
		}		
		
		if (strstr(theitems3[cnt], "Actual") != NULL) { 
			if  ( (strstr(theitems3[cnt+1], "Alert") != NULL) || (strstr(theitems3[cnt+1], "Update") != NULL) ||
				(strstr(theitems3[cnt+1], "Cancel") != NULL) ) { 
				if (strstr(theitems3[cnt+5], "Moderate") != NULL) {
					threatlevel = 2;
					if (strstr(theitems3[cnt+8], "event") != NULL) {
						strcpy(the_event, theitems3[cnt+8]);
					}	
					if ( (s != 1) || ( (s == 1) && (TL2 == 1) ) ) {
						if (i != 1) {
							actual_printer(a, cnt, newcnt); 
						}
						else {
							imgmaker(a);
						}
						newcnt++; 
					}
				}
			}	 
		}
	
		if ( (strstr(theitems3[cnt], "Actual") != NULL) || (strstr(theitems3[cnt], "Exercise") != NULL) ) { 
				if (strstr(theitems3[cnt+5], "Minor") != NULL) {
					threatlevel = 3;
					if (strstr(theitems3[cnt+8], "event") != NULL) {
						strcpy(the_event, theitems3[cnt+8]);
					}
					if ( (s != 1) || ( (s == 1) && (TL3 == 1) ) ) {
						if (i != 1) {
							actual_printer(a, cnt, newcnt); 
						}
						else {
							imgmaker(a);
						}
						newcnt++;
					} 
				}
		} 
	
		if ( (strstr(theitems3[cnt], "Exercise") != NULL) || (strstr(theitems3[cnt], "System") != NULL) ||
			(strstr(theitems3[cnt], "Test") != NULL) ) { 
			threatlevel = 4;
			if (strstr(theitems3[cnt+8], "event") != NULL) {
				strcpy(the_event, theitems3[cnt+8]);
			}
			if ( (s != 1) || ( (s == 1) && (TL4 == 1) ) ) {
				if (i != 1) {
					actual_printer(a, cnt, newcnt); 
				}
				else {
					imgmaker(a);
				}
				newcnt++; 
			}
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
	int a = 0;
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
	exit(EXIT_SUCCESS);
}

static int flag_help;

static struct option const long_options[] = 
{
    	{"imagegenerate", no_argument, 0, 'i'}, 
	{"help", no_argument, &flag_help, 1},
	{"custom", no_argument, 0, 'c'},
	{"select", no_argument, 0, 's'},
	{"download", no_argument, 0, 'd'}, 
   	{"twitter", no_argument, 0, 't'},
	{0, 0, 0, 0}			
};

void usage() {
	printf("usage:\ntl (list alert items)\ntl -s [1] [2] [3] [4] (select individual threat levels)\ntl -i (generate alert images)\ntl -d (download new xml file)\ntl -t (post to twitter)\ntl -c (create custom image)\n");
	if (flag_help) 
		exit(EXIT_SUCCESS);
	else
		exit(EXIT_FAILURE);
}

int main (int argc, char* argv[]) {
	int optc;
	int index;

	// test for imagemagick before starting

	if ((access("/usr/bin/convert", F_OK) == -1)) {
		printf("imagemagick not installed\n");
		exit(EXIT_FAILURE);
	}

	while ((optc = getopt_long (argc, argv, "idcst", long_options, (int *) 0)) != EOF) {
		switch (optc) {
			case 0:
		  	    break; 
			case 'i':
	          	    i = 1;
		  	    break;
			case 'd':
		  	    d = 1;
		  	    break;
			case 'c':
		            c = 1;
		  	    break;
			case 's':
		            s = 1;
		  	    break;
			case 't':
		 	    t = 1;
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
		leftover = index - optind;

		// first check if we are only printing specific threatlevels
	
		if ( ((leftover > 0) && (s == 1)) && ((c == 0) && (d == 0)) ) {
			if (argc > 4) {
				printf("too many arguments\n");		
				usage();	
			}
			
			int qq;
			int tester;
			// check if specific entered threatlevel is indeed an INT
			for (qq = 0; qq < argc; qq++) {
				tester = atoi(argv[qq]);
				
				if ( (tester > 4) || (tester < 1) ) {
					printf("Err. Numbers must be 1 - 4\n");
					usage();
				}
				// if TLX set, then print that(s) rather than all threats
				if (tester == 1) {
					TL1 = 1;
				}
				if (tester == 2) {
					TL2 = 1;
				}
				if (tester == 3) {
					TL3 = 1;
				}
				if (tester == 4) {
					TL4 = 1;
				}
			}
		//printf("TL1: %d TL2: %d TL3: %d TL4:%d\n", TL1, TL2, TL3, TL4);
		thefunc(argc, argv);
		}	

		if ((leftover == 0) && ((i == 1) || (t == 1))) {
			thefunc(argc, argv);
		}
		if ((leftover == 0) && (c == 1) && (s == 0))  {
			customimg();
		}
		if ((leftover == 0) && (i == 0) && (d == 1) && (s == 0)) {
			curl_func();
			exit(EXIT_SUCCESS);	
		}
		else {
			usage();
		}
	}
}
