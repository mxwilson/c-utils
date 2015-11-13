// ctask2 - simple task manager using mysql.
// update the global vars below to intergrate with your install of mysql.
// matthew wilson (c) 2015
// License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>
// No warranty. Software provided as is.
// requires mysql and libmysqlclient-dev 

// to compile:
// gcc ctask2.c -o ctask2 $(mysql_config --libs --cflags)

#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include <my_global.h>
#include <mysql.h>
#define ANSI_RED "\x1b[31m"
#define ANSI_GREEN "\x1b[32m"
#define ANSI_ORANGE "\x1b[33m"
#define ANSI_RESET "\x1b[0m"

// global vars

char* database_name = "thenewdb";
char* table_name = "ctasklist";
char* db_hostname = "localhost";
char* db_username = "yourusername";
char* db_password = "yourpassword";

// prototypes

void create_table(MYSQL* con);
void createdb();
void the_lister(MYSQL* con);
void dbtester(int argc, char* argv[], char* newwstring);
void new_addition(MYSQL* con, int argc, char* argv[], char* newwstring);
void the_deleter(MYSQL* con, int argc, char* argv[]);
void thefunc(int argc, char*argv[]);
void usage();

// error handler function

void err_handler(MYSQL *con) {
printf("%s\n", mysql_error(con));
mysql_close(con);
exit(1);
}

// this is main function. then call the dbtester

int leftover=0;
int l=0, n=0, d=0;

void thefunc(int argc, char*argv[]) {
char* newwstring;
int a;
unsigned int stringsize=0;

//printf("l=%d n=%d d=%d\n", l, n, d);

// before testing db, going to concat any strings from the command line

if (leftover > 1 && n == 1) {

	for (a=0; a < argc; a++) {
		stringsize += strlen(argv[a])+1;
	}	
	
	malloc(stringsize)+1;
	newwstring = (char *) malloc(1 + sizeof(char*) * (stringsize));
	
	for (a=0; a < argc; a++) {
		strcat(newwstring, argv[a]);

		if (a < (argc - 1)) {
			strcat(newwstring, " ");
		}
	}
}

dbtester(argc, argv, newwstring);

}

void dbtester(int argc, char*argv[], char*newwstring) {

// now begin testing db

MYSQL* con = mysql_init(NULL);

if (con == NULL) {
        printf("%s\n", mysql_error(con));
        exit(1);
}

// first connect and login to the db

if (mysql_real_connect(con, db_hostname, db_username, db_password, NULL, 0, NULL, 0) == NULL) {
        printf("%s\n", mysql_error(con));
        mysql_close(con);
        exit(1);
}

// possibly create the db

char create_db_buf[50];

if (mysql_select_db(con, database_name) != 0) {
        printf("Creating DB.\n");
        snprintf(create_db_buf, sizeof create_db_buf, "CREATE DATABASE IF NOT EXISTS %s", database_name);

        if (mysql_query(con, create_db_buf)) {
                err_handler(con);
        }
}

// then select the db to use

char lilbuf[20];

snprintf(lilbuf, sizeof lilbuf, "USE %s", database_name);

if (mysql_query(con, lilbuf)) {
        err_handler(con);
}

// then create the table if not exist. id set to auto increment and will insert automatically

char tablcrbuff[350];

snprintf(tablcrbuff, sizeof tablcrbuff, "CREATE TABLE IF NOT EXISTS %s(id SMALLINT NOT NULL PRIMARY KEY AUTO_INCREMENT, taskname CHAR(128), taskdesc CHAR(128), priority INT, created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP)", table_name);

mysql_query(con, tablcrbuff);

//  now that db is tested, call various functions

if (l == 1 && n == 0 & d == 0) {
	the_lister(con);
}

if (n == 1 && l ==0 && d == 0) {
	new_addition(con, argc, argv, newwstring);

}

if (d == 1 & l == 0 && n == 0) {
	the_deleter(con, argc, argv);
}

else {
	the_lister(con);
}

}

// deleting function

void the_deleter(MYSQL* con, int argc, char* argv[]) {
char delbuf[50];
int delnum;

// either ask task to delete or use task# from command line

if (leftover == 0) {
	printf("ID of task to delete: ");
	fscanf(stdin, "%d", &delnum);
}

else {
	delnum=atoi(argv[0]);	
}

// construct the query

snprintf(delbuf, sizeof delbuf, "DELETE FROM %s WHERE id=(%d)", table_name, delnum);
mysql_query(con, delbuf);

// using mysql_affected_rows tells if success

if ((long)mysql_affected_rows(con) != 0) {		
	printf("Task %d deleted.\n", delnum);
}

else {
	printf("Task ID not found.\n");
}

mysql_close(con);
exit(0);
}

// new task function

void new_addition(MYSQL* con, int argc, char* argv[], char* newwstring) {
char newtaskname[60];
char newdescname[150];
char embuff[150];
int inputnum;
int num;
int ln;

// again, either ask for new task name or take from command line

if (leftover == 0) {
	printf("Enter name of new task: ");
	fgets(newtaskname, 60, stdin);
}

else if (leftover > 1) {
	strcpy(newtaskname, newwstring);
}

// remove newline

ln = strlen(newtaskname) - 1;

if (newtaskname[ln] == '\n') {
	newtaskname[ln] = '\0';
}

printf("Enter description: ");
fgets(newdescname, 150, stdin);

ln = strlen(newdescname) - 1;

if (newdescname[ln] == '\n') {
	newdescname[ln] = '\0';
}

// error check priority for 1-3

do {
	printf("Priority (1-3): ");
	if (fscanf(stdin, "%d", &inputnum) == 1) {
		if (inputnum >= 1 && inputnum <= 3) 
			break;
	}
	else {
		while ((inputnum = getchar()) != '\n' && inputnum != EOF);
	}
	
} while(1);

// make the sql statement and run the query below

snprintf(embuff, sizeof embuff, "INSERT INTO %s(taskname, taskdesc, priority) VALUES(\"%s\", \"%s\", %d)", table_name, newtaskname, newdescname, inputnum);

printf("Task %s added!\n", newtaskname);

if (mysql_query(con, embuff)) {
       err_handler(con);
}

mysql_close(con);
exit(0);
}

// list tasks from the db

void the_lister(MYSQL* con) {
char selectbuf[30];

snprintf(selectbuf, sizeof selectbuf, "SELECT * FROM %s", table_name);

if (mysql_query(con, selectbuf)) {
        err_handler(con);
}

// mysql_store_result = loads all rows into memory

MYSQL_RES *result = mysql_store_result(con);

if (result == NULL) {
        err_handler(con);
}

int num_fields = mysql_num_fields(result);

// count the rows. exit early if none 

int row_count = mysql_num_rows(result);

if (row_count == 0) {
	printf("DB empty, exit.\n");
	mysql_close(con);	
	usage();
}

MYSQL_ROW row;
int i;

// each val within the row can be accessed as row[i] where i starts at 0
// to the num of columns in row minus 1

while ((row = mysql_fetch_row(result))) {
       	printf("[%s] ", row[0]);
	if ((atoi(row[3]) == 1)) {
		printf(ANSI_RED "%s (P1)" ANSI_RESET, row[1]);
	}
       	else if ((atoi(row[3]) == 2)) {
                printf(ANSI_ORANGE "%s (P2)" ANSI_RESET, row[1]);
       	}
       	else if ((atoi(row[3]) == 3)) {
                printf(ANSI_GREEN "%s (P3)" ANSI_RESET, row[1]);
       	}
	printf(" | added: %s\n\n", row[4]);
	printf("    %s\n\n", row[2]);    	
}

printf("Tasks in DB: %d\n", row_count);

// free result and close connection

mysql_free_result(result);
mysql_close(con);
exit(0);
}

static int flag_help;

static struct option const long_options[] = 
{
	{"l", no_argument, 0, 'l'}, 
    	{"n", no_argument, 0, 'n'}, 
    	{"d", no_argument, 0, 'd'},
	{"help", no_argument, &flag_help, 1}, 
   	{0, 0, 0, 0}			
};

void usage() {
printf("usage: ctask2 -l (list) | -n (new)[taskname] | -d (delete)[taskid]\n");
exit(0);
}

int main (int argc, char* argv[]) {
int optc;
int index;

while ((optc = getopt_long (argc, argv, "lnd", long_options, (int *) 0)) != EOF) {
	switch (optc) {
	    case 0:
		break; 
	    case 'l':
	        l=1;
		break;
	    case 'n':
	        n=1;
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

if (argc == 1) {
	// default, will list
	l=1;
	thefunc(argc, argv);
}

else if (argc > 1) {
	for (index=optind; index<argc; index++) {
		;
	}

	argc -= optind;
	argv += optind;
	leftover=index-optind;
	thefunc(argc, argv);
}

}
