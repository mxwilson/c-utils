#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<time.h>
#include<utmp.h>
#include<sys/types.h>
#include<string.h>
#include<fcntl.h>

// login-notify.c - v.0.1.2 (c) Matthew Wilson. 2015. 
// SSH log-in notifier. Uses SSMTP/mailx to send email upon new login.
// Program is run by cron job. 
// License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>
// No warranty. Software provided as is.

thefunc(int argcc, char*argvv[]) {
struct utmp ii;
time_t logintime_raw;
char tempbuf[99];
int linesold=0;
int ln=0;
char* wtmplogfile="/var/log/wtmp"; // location of wtmp log file
char* comparelogfile="./notlog.log"; // location of compare log file 
char* emailaddy="user@email.com"; // email address for notifications

// going to filter out all results starting with tty in the wtmp,
// and put them in the items[array].
// if the old log file/compare file is not present, it will create one.
// lines are counted and compared in old and new logs.
// if new log has more lines, get those lines and mail them.
// delete old log file and re-create including new lines.

char* TTYcheck=NULL;
FILE *LOGfp;
FILE *pfo;
int oldlogexist=0;
char buff[999];
char embuff[999];
char* items[2048];
char* emailitems[2048];

if (access(comparelogfile, F_OK|W_OK) == 0) {
	oldlogexist=1;
}

LOGfp = fopen(wtmplogfile, "r");

if (LOGfp == NULL) {
	printf("Unable to open wtmp log.\n");
	exit(1);
}
	
while (fread(&ii, sizeof ii, 1, LOGfp) !=0) {
	if (ii.ut_type==7) {
        	// look for logins other than tty
		TTYcheck = strstr(ii.ut_line, "tty");
		
		if (TTYcheck != ii.ut_line) {
             		logintime_raw=ii.ut_time;
                        strcpy(tempbuf, ctime(&logintime_raw));
                        tempbuf[strlen(tempbuf)-1]='\0';
			
			if (oldlogexist != 1) {
				pfo=fopen(comparelogfile, "a+");
			}                       

			if (strlen(ii.ut_host) > 4 )  {
				snprintf(buff, sizeof buff, "%s %s %s %s", ii.ut_user, tempbuf, ii.ut_line, ii.ut_host);
				items[ln]=malloc(strlen(buff)+1);
				strcpy(items[ln], buff);
				//printf("%d: %s\n", ln, items[ln]);	
			
				if (oldlogexist != 1) {
					fprintf(pfo, "%s ", ii.ut_user);
                               		fprintf(pfo, "%s ", tempbuf);
                                	fprintf(pfo, "%s ", ii.ut_line);
                                	fprintf(pfo, "%s ", ii.ut_host);
			        	fprintf(pfo, "\n");
				}				
			ln++; // counter 
			}
		}
	}
}

fclose(LOGfp);

if (oldlogexist != 1) {
	fclose(pfo);
	exit(0);
}

// now count lines in log file
if (oldlogexist != 0) {
	int c;
	FILE *fp0;

	if (!(fp0=fopen(comparelogfile, "r"))) {
		printf("line count file error\n");
		exit(1);
	}
	else {
		while ((c=fgetc(fp0)) != EOF) {
			if (c == '\n') {
				++linesold;
			}
		}
	fclose(fp0);
	}
	
	// compare lines in each log
	int thediff=0;
	int x;

	if (ln > linesold) {
		thediff=(ln - linesold);
		
		// list of diffs to be emailed in emailitems[arr]
		// below is command to run mailx
		for (x=(ln-thediff); x<ln;  x++) {
			emailitems[x]=malloc(strlen(items[x])+1);
			strcpy(emailitems[x], items[x]);
			snprintf(embuff, sizeof embuff, "echo \"%s\" | mailx -s \"login\" %s", emailitems[x], emailaddy);
			system(embuff);
			//printf("diff: %s\n", embuff);
		}
		// delete old and create new log file
		if (access(comparelogfile, F_OK|W_OK) != -1) {
			remove(comparelogfile);
			FILE* pfoN;	
			pfoN=fopen(comparelogfile, "a+");
	        	
			for (x=ln; x-- > 0;) {				
				fprintf(pfoN, "%s\n", items[x]);
			}
			fclose(pfoN);
		}
	}
}

exit(0);
}

main (int argc, char* argv[]) {
	if (argc == 1) {
		thefunc(argc, argv);
	}
	else {
		printf("err\n");
		exit(1);
	}
}
