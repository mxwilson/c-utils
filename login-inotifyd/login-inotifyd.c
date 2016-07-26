// <login-inotifyd.c> - e-mail ssh login notifications using mailx.
// v 0.1.6. Copyright 2015-16, Matthew Wilson. 
// License GPLv3+: GNU GPL version 3 or later: http://gnu.org/licenses/gpl.html
// No warranty. Software provided as is.

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/inotify.h>
#include<sys/stat.h>
#include<unistd.h>
#include<time.h>
#include<utmp.h>
#include<fcntl.h>
#include<signal.h>
#include<sys/types.h>
#include<signal.h>

char* emailaddy="yourname@email.com"; // e-mail address for notifications

#define LOCK_FILE "/var/lock/login-inotifyd.lock" // lock file location
#define PID_FILE "/var/run/login-inotifyd.pid" // pid file location
#define EVENT_SIZE (sizeof(struct inotify_event))
#define EVENT_BUF_LEN (1024*(EVENT_SIZE + 16))
char* errlog="/var/log/login-inotifyd.log"; // program log file
char* wtmplogfile="/var/log/wtmp"; // location of wtmp log file
int ln=0;
int cntrr;
int onetime_cntrr;
char* itemtoemail; // body of e-mail text
char* untoemail; // username of logged in user to e-mail

// prototypes

int inotify_function();
int get_event(struct inotify_event *event);
int logger(char* message);
void signal_handler(int sig);

// function to read wtmp log file

int logfilereader() {
char* items[2048];
struct utmp ii;
time_t logintime_raw;
char tempbuf[99];
char* TTYcheck=NULL;
char buff[999];
FILE *LOGfp;
LOGfp = fopen(wtmplogfile, "r");

if (LOGfp == NULL) {
        logger("Unable to open wtmp log file. Program exit.");
        exit(1);
}

// check for type 7 (login) and not a tty login, and host is not tmux or an xterm
// look for remote/ssh login. create buff to send in e-mail. and also untoemail to
// pass just username in e-mail subject line

while (fread(&ii, sizeof ii, 1, LOGfp) != 0) {
	if (ii.ut_type == 7) {
        	if ( (! strstr(ii.ut_line, "tty")) && (! strstr(ii.ut_host, "tmux")) && 
			(! strstr(ii.ut_host, "localhost")) && (! strstr(ii.ut_host, ":0")) ) {
			logintime_raw=ii.ut_time;
                        strcpy(tempbuf, ctime(&logintime_raw));
                        tempbuf[strlen(tempbuf) - 1] = '\0';
			snprintf(buff, sizeof buff, "%s %s %s %s", ii.ut_user, tempbuf, ii.ut_line, ii.ut_host);
                        items[ln]=malloc(strlen(buff) + 1);
                        strcpy(items[ln], buff);
			untoemail=malloc(strlen(ii.ut_user) + 1);
			strcpy(untoemail, ii.ut_user);
			ln++;
		}
	}
}


fclose(LOGfp);

// if no ssh logins in wtmp log upon startup, resume program
if (ln == 0) {
	inotify_function();
}

// get most recent item in wtmp and assign to itemtoemail
else {
	cntrr=(ln - 1);
	itemtoemail=malloc(strlen(items[cntrr]) + 1); 
	strcpy(itemtoemail, items[cntrr]);
	items[0] = '\0';
	ln=0;
}

}

// "main" portion of the program 

int inotify_function() {
long length;
int fd;
int wd; // watch descriptor
char* thefile;
char* ptr;
char buffer[EVENT_BUF_LEN];
struct inotify_event *event;

// create instance of inotify

fd=inotify_init();

// check for err

if (fd < 0) {
	logger("Inotify error.\n");
	exit(1);
}

// watch descriptor - watch all modified items in /var/log

wd = inotify_add_watch(fd, "/var/log", IN_MODIFY);

// check for err

if (wd < 0) {
	logger("Error reading /var/log directory.\n");
	exit(1);
}

// wait for change to occur

while(1) {
	length = read(fd, buffer, EVENT_BUF_LEN);
	
	if (length <= 0) {
		logger("Error reading incoming events.\n");
		exit(1);
	}
	
	ptr=buffer;

	while (ptr < buffer + length) {
		event=(struct inotify_event *) ptr;
		get_event(event); // now call the event handler below
		ptr += sizeof(struct inotify_event) + event->len;
	}
}

}

// incoming event handler

int get_event(struct inotify_event *event) {

char embuff[300];

if (event->len > 0) {
	// check for event, specifically when wtmp file is modified
	if (strcmp(event->name, "wtmp") == 0) {

		// then call log reader
		logfilereader();		
		
		// login & logout produce similar event. capture only once with a one time counter. 
		// then create an e-mail with subject and body and call mailx
		if (onetime_cntrr != cntrr) {
			snprintf(embuff, sizeof embuff, "echo \"%s\" | mailx -s \"user login: %s\" %s", itemtoemail, untoemail, emailaddy);
			system(embuff);
			logger(itemtoemail); // also write to log
		}
		onetime_cntrr=cntrr;
	}
}

}

// program log file function

int logger(char* message) {
char timebuf[33];
time_t thetime;
thetime = time(NULL);
strcpy(timebuf, ctime(&thetime));
timebuf[strlen(timebuf) - 1] = '\0';

FILE* errlogfp;
errlogfp=fopen(errlog, "a");
fprintf(errlogfp, "%s: %s\n", timebuf, message);
fclose(errlogfp);
}

// signal handler function. removes lock file on exit.

int sig;
void signal_handler(int sig) {
switch(sig) {
        case SIGHUP:
                logger("Hangup signal. Program exit.");
                if (access(LOCK_FILE, F_OK) == 0) {
                        remove(LOCK_FILE);
                }
		if (access(PID_FILE, F_OK) == 0) {
                        remove(PID_FILE);
                }
                exit(0);
                break;
        case SIGTERM:
                logger("Terminate signal. Program exit.");
                if (access(LOCK_FILE, F_OK) == 0) {
                        remove(LOCK_FILE);
                }
		if (access(PID_FILE, F_OK) == 0) {
                        remove(PID_FILE);
                }
                exit(0);
                break;
        }
}

// forking function

int forker() {

int lfp;
int pfp;
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

lfp=open(LOCK_FILE, O_RDWR|O_CREAT|O_CLOEXEC, 0644);

if (lfp < 0) {
        logger("Lock file error. Program exit.");
        exit(1);
}

if (lockf(lfp, F_TLOCK, 0) < 0) {
        printf("Program already running.\n");
        exit(0);
}

// and pid file

pfp=open(PID_FILE, O_RDWR|O_CREAT|O_CLOEXEC, 0644);

if (pfp < 0) {
        logger("PID file error. Program exit.");
        exit(1);
}

// continue, write pid to lock file

sprintf(str, "%d\n", getpid());
write(lfp, str, strlen(str));
write(pfp, str, strlen(str));

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
        inotify_function();
}

}

void main(int argc, char* argv[]) {

if (access("/usr/bin/mailx", F_OK) == -1) {
        logger("Error: mailx not installed at: /usr/bin/mailx - Install GNU Mailutils.");
        exit(1);
}

if (argc != 1) {
	printf("Error: must be run as a daemon.\n");
}

else {
	// fork before calling main program
	forker();
}
}
