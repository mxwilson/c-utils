// login-notifyd.c - v.0.1 (c) Matthew Wilson. 2015. 
// SSH log-in notifier daemon. Uses SSMTP/mailx to send email upon new login.
// Install included Systemd service file to run.  
// License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>
// No warranty. Software provided as is.

#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<time.h>
#include<utmp.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<string.h>
#include<fcntl.h>
#include<signal.h>
#define LOCK_FILE "/run/lock/login-notifyd.lock" // lock file location
#define SLEEPYTIME 30 // time of heartbeat in seconds

char* emailaddy="username@email.com"; // email address for notifications
char* wtmplogfile="/var/log/wtmp"; // location of wtmp log file
char* comparelogfile="/var/log/login-notifyd.log"; // compare log file 
char* errlog="/var/log/login-notifyd-err.log"; // program log file
char* message;

// program log file function

void logger(message) {
char timebuf[33];
time_t thetime;
thetime = time(NULL);
strcpy(timebuf, ctime(&thetime));
timebuf[strlen(timebuf)-1]='\0';

FILE* errlogfp;
errlogfp=fopen(errlog, "a");	
fprintf(errlogfp, "%s: %s\n", timebuf, message);
fclose(errlogfp);
}

// signal handler function. removes lock file on exit.

int sig;
void signal_handler(sig) {
switch(sig) {
	case SIGHUP:
		logger("Hangup signal. Program exit.");
		if (access(LOCK_FILE, F_OK) == 0) {
                        remove(LOCK_FILE);
                }
		exit(0);
		break;
	case SIGTERM:
		logger("Terminate signal. Program exit.");
		if (access(LOCK_FILE, F_OK) == 0) {
        		remove(LOCK_FILE);
		}
		exit(0);
		break;
	}
}

// main program 

thefunc() {
struct utmp ii;
time_t logintime_raw;
char tempbuf[99];
int linesold=0;
int ln=0;
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
	logger("Unable to open wtmp log file. Program exit.");
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
}

// now count lines in compare log file

if (oldlogexist != 0) {
	int c;
	FILE *fp0;

	if (!(fp0=fopen(comparelogfile, "r"))) {
		logger("Unable to open compare log file. Program exit.");
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
			logger("Emailing diff.");
			system(embuff);
		}
		// delete old and create new log file
		if (access(comparelogfile, F_OK|W_OK) != -1) {
			remove(comparelogfile);
			
			FILE* pfoN;	

			if (!(pfoN=fopen(comparelogfile, "a+"))) {
				logger("Compare log create err. Program exit.");
				exit(1);
			}
 			for (x=ln; x-- > 0;) {				
				fprintf(pfoN, "%s\n", items[x]);
			}
			fclose(pfoN);
		}
	}
}

}

main (int argc, char* argv[]) {

if (argc > 1) {
	printf("Too many arguments. Program exit.\n");
	exit(1);
}

// begin forking

int lfp;
char str[10];

pid_t processid = 0;
pid_t sid = 0;

processid=fork();

if (processid < 0) {
	logger("Fork error. Program exit.");
	exit(1);
}

if (processid > 0) {
	exit(0);
}

umask(0);

sid=setsid();

if (sid < 0) {
	exit(1);
}

chdir("/");

// now check for lock file

lfp=open(LOCK_FILE, O_RDWR|O_CREAT|O_CLOEXEC, 0640);

if (lfp < 0) {
	logger("Lock file error. Program exit.");
	exit(1); 
}

if (lockf(lfp, F_TLOCK, 0) < 0) {
	printf("Program already running.\n");
	exit(0); 
}

// continue, write pid to lock file

sprintf(str, "%d\n", getpid());
write(lfp, str, strlen(str));

// close i/o

close(STDIN_FILENO);
close(STDOUT_FILENO);
close(STDERR_FILENO);

// check for signals function

signal(SIGHUP, signal_handler); 
signal(SIGTERM, signal_handler);  

// now call main program

logger("Program start OK.");

while(1) {
	thefunc();
	sleep(SLEEPYTIME);
}

}
