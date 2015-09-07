/* CTASK v0.1 */
// ctask is a simple task manager using a flat file db. 
// Inspired by CD's todo at <https://github.com/ddddddeon/todo>
// License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>
// No warranty. Software provided as is.
// Copyright Matthew Wilson, 2015.

#include<stdio.h>
#include<time.h>
#include<getopt.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/stat.h>
#define DBFILE "./db.dat" // location of db file
#define ANSI_RED 	"\x1b[31m"
#define ANSI_GREEN 	"\x1b[32m"
#define ANSI_ORANGE 	"\x1b[33m"
#define ANSI_RESET	"\x1b[0m"

int l=0, n=0, p=0, d=0;
int linecount=0;

// file reading function

filereader() {
char line[400];
char* lineitems[800];
FILE* fp;
// vars below increased to 800: 2426 lines, 1000: 2626 lines
char* item1[500]; // task name
char* item2[500]; // description
char* item3[500];  // task priority
char* timeadded[500]; // time added
char* delim=":";

if (linecount == 0) {
	printf("No lines in db! ;)\n");
	exit(0);
}

// now knowing db file is not empty, capture each line and tokenize

if (!(fp=fopen(DBFILE, "r"))) {
	printf("DB file read error :(\n");
	exit(1);
}

int x;

for (x=0; x<linecount; x++) {
	fgets(line, sizeof line, fp);
	lineitems[x]=malloc(strlen(line)+1);
	lineitems[x]=strdup(line);
	item1[x]=strtok(lineitems[x], delim);
	item2[x]=strtok(NULL, delim);
	item3[x]=strtok(NULL, delim);
	timeadded[x]=strtok(NULL, delim);		
	
	// then print 

	printf("[%d] ", x+1);
        if ((atoi(item3[x]) == 1)) {
		printf(ANSI_RED "%s (P1)" ANSI_RESET, item1[x]);
	}
        else if ((atoi(item3[x]) == 2)) {
                printf(ANSI_ORANGE "%s (P2)" ANSI_RESET, item1[x]);
        }
        else if ((atoi(item3[x]) == 3)) {
                printf(ANSI_GREEN "%s (P3)" ANSI_RESET, item1[x]);
        }
		printf(" | added: %s\n", timeadded[x]);
		printf("     %s\n\n", item2[x]);

}

fclose(fp);
exit(0);
}

// New task function

newtask(int argcc, char*argvv[], int incoming_optarg) {
FILE* fp;
char newtaskname[51];
char newdesc[101];
int priority;
int num;
int inputnum;
int a;
unsigned int stringsize=0;
int ln;
int ln2;

// check for errors if priority specified on command line

inputnum=incoming_optarg;

if (p == 1 && (inputnum > 3 || inputnum < 1)) {
	printf("Priority value out of range. Must be 1-3\n");
	exit(1);
}

if (argcc == 0) {
	printf("Enter name of new task: ");	
	fgets(newtaskname, 50, stdin);

	ln = strlen(newtaskname) - 1;
	if (newtaskname[ln] == '\n') {
    		newtaskname[ln] = '\0';
	}
}

// concat all arg vectors into one string

else {			
	for (a=0; a<argcc; a++) {
		stringsize += strlen(argvv[a])+1;
	}
	
	malloc(stringsize)+1;

	char * str3 = (char *) malloc(1 +sizeof(char*) * (stringsize));	

	for (a=0; a<argcc; a++) {
		strcat(str3, argvv[a]);	
	
		if (a < (argcc-1)) {
			strcat(str3, " ");
		}
	}		
	strcpy(newtaskname, str3);
}

printf("Description: ");
fgets(newdesc, 100, stdin);
ln2 = strlen(newdesc) - 1;

if (newdesc[ln2] == '\n') {
    	newdesc[ln2] = '\0';
}

// task priority. either take from command line or prompt

if (p != 1) { 
	do {
		printf("Priority (1-3): ");
		if (fscanf(stdin, "%d", &inputnum) == 1) {
			if (inputnum <= 3 && inputnum >= 1) 	
				break;
			}
		else {
			while ((num=getchar()) != '\n' && num != EOF);
		}
	} while (1);
}


// grab the time 

struct tm *info; 
time_t curtime;
curtime=time(NULL);
info=localtime(&curtime);
int year=info->tm_year+1900;
int mon=info->tm_mon+1;
char timeline[50];

snprintf(timeline, sizeof timeline, "%d-%d-%d-%dh-%02dm", info->tm_mday, mon, year, info->tm_hour, info->tm_min);

// now write to db

if (!(fp=fopen(DBFILE, "a"))) {
	printf("output file error\n");
	exit(1);
}

char lineadd[300];
snprintf(lineadd, sizeof lineadd, "%s:%s:%d:%s\n", newtaskname, newdesc, inputnum, timeline);
fputs(lineadd, fp);
printf("Task %s added.\n", newtaskname);
fclose(fp);
	
exit(0);
}

// delete task function

deletetask(char*argvv[], int argcc) {
FILE* fp;
int num;
int inputnum;

if (argcc > 1) {
	printf("Too many args :(\n");
	exit(1);
}

if (linecount == 0) {
	printf("No lines in db!\n");
	exit(0);
}

if (argcc == 1) {
	inputnum=atoi(argvv[0]);

	if (inputnum > linecount) {
		printf("No such line in DB.\n"); 
		exit(1);
	}
}

// figure out what argcc is and delete the line

if (argcc == 0) {
	do {
		printf("Task number to delete: ");
		if (fscanf(stdin, "%d", &inputnum) == 1) {
			if ( (inputnum <= linecount) && (inputnum > 0) ) 	
				break;
			}
			else {
				while ((num=getchar()) != '\n' && num != EOF);
			}
	} while (1);
}

// now delete. going to read in all db items. then re-write db file without
// line specified

char* lineitems[500];
char line[400];
int x;

fp=fopen(DBFILE, "r");

for (x=0; x<linecount; x++) {
	fgets(line, sizeof line, fp);
	lineitems[x]=malloc(strlen(line)+1);
	lineitems[x]=strdup(line);
}

fclose(fp);

// now write to db file minus deleted line

fp=fopen(DBFILE, "w");

for (x=0; x < inputnum-1; x++) {
	fputs(lineitems[x], fp);
}

for (x=inputnum; x < linecount; x++) {
	fputs(lineitems[x], fp);
}

fclose(fp);
printf("Deleted task %d\n", inputnum);

exit(0);
}

// line counter/db create function

linecounter(int argcc) {
FILE* fp;
int c;

// create db file if not exist.

if (access(DBFILE, F_OK|W_OK) == -1) {
	creat(DBFILE, S_IRWXU);
	printf("No DB file. Creating one :)\n");

	if (argcc == 0) {
		exit(0);
	}
}

if (!(fp=fopen(DBFILE, "r"))) {
	printf("DB file read error :(\n");
	exit(1);
}

// count lines

while ((c=fgetc(fp)) != EOF) {
	if (c == '\n') {
		++linecount;
	}
}

fclose(fp);
}

thefunc(int argcc, char*argvv[], int incoming_optarg) {

// first count lines in the db file or create it

linecounter(argcc);

// then decide what action to take

// no args or -l will list all tasks

if ( (argcc == 0 && n != 1 && p != 1 && d != 1) ||
   (argcc > 0 && l == 0 && n == 0 && p == 0 && d == 0) ) { 
	filereader();
}

// add a new task -n

if (n == 1 && l != 1 && d != 1) {
	newtask(argcc, argvv, incoming_optarg);
}

// delete task

if (d == 1 && l != 1 && n != 1 && p != 1) {
	deletetask(argvv, argcc);
}

exit(0);
}

static int flag_help;

static struct option const long_options[] = 
{
	{"list", no_argument, 0, 'l'}, 
    	{"new", no_argument, 0, 'n'}, 
    	{"priority", no_argument, 0, 'p'},
	{"delete", no_argument, 0, 'd'},
	{"help", no_argument, &flag_help, 1}, 
   	{0, 0, 0, 0}			
};

usage() {
printf("usage:\nctask\n-l :list\n-n :new [task name] -p :priority [1-3]\n-d :delete [task #]\n");
exit(1);
}

main (int argc, char* argv[]) {
int optc;
int index;
extern char* optarg;
int incoming_optarg;

while ((optc = getopt_long (argc, argv, "lnp:d", long_options, (int *) 0)) !=EOF)
  {
	switch (optc) {
		case 0:
			break; 
		case 'l':
			l=1;
			break;
		case 'n':
			n=1;
			break;
		case 'p':
			p=1;
			incoming_optarg=atoi(optarg);	
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
	usage();
}

for (index=optind; index<argc; index++) {
	;
}

argc -= optind;
argv += optind;

// call the first function to decide action

thefunc(argc, argv, incoming_optarg);

}
