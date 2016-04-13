// <login-inotifyd.c> - e-mail ssh and sftp login notifications using mailx and ssmtp
// v 0.2. Copyright 2015-16, Matthew Wilson. 
// License GPLv3+: GNU GPL version 3 or later: http://gnu.org/licenses/gpl.html
// No warranty. Software provided as is.

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/inotify.h>
#include<sys/stat.h>
#include<unistd.h>
#include<time.h>
#include<fcntl.h>
#include<signal.h>
#include<sys/types.h>

char* emailaddy="yourname@email.com"; // e-mail address for notifications
char* proglog="/var/log/login-inotifyd.log"; // program log file
char* logfile="/var/log/auth.log"; // location auth.log file
#define LOCK_FILE "/var/lock/login-inotifyd.lock" // lock file location
#define PID_FILE "/var/run/login-inotifyd.pid" // pid file location
#define EVENT_SIZE (sizeof(struct inotify_event))
#define EVENT_BUF_LEN (1024*(EVENT_SIZE + 16))
char* msgitem[2]; // items to send in email

// prototypes

int inotify_function();
int get_event(struct inotify_event *event);
int logger(char* message);
void signal_handler(int sig);

// function to read auth.log file

int logfilereader() {

FILE* f;
char* lineitems[2000];
char line[300];
int x=0;
int a;
int linenum=0;

char* parstime;
char* parslogentry;
char* logentrycopy;
char* parsusr;
char* ptr = NULL;
char* ptr2 = NULL;

f = fopen(logfile, "r");

if (f == NULL) {
	printf("unable to open file\n");
	exit(1);
}

while (fgets(line, sizeof(line) - 1, f) != NULL) {
	lineitems[x] = malloc(strlen(line) + 1);
	strcpy(lineitems[x], line);
	lineitems[x][strcspn(lineitems[x], "\n")] = 0;
	x++;
}

// only grab the last line in the file and test

linenum = (x-1);	

if (strstr(lineitems[linenum], "Accepted") != NULL) {
	// get date and time malloc 15 chars + 1. copy @ posit. 0-15
	parstime = malloc(15 + 1);
	strncpy(parstime, lineitems[linenum] + 0, 15);

	// tokenlize lineitems until reaching word "Accepted"
	ptr = strtok(lineitems[linenum], ":");
	ptr = strtok(NULL, ":");
	ptr = strtok(NULL, ":");
	ptr = strtok(NULL, ";"); // allows capture of localhost ssh login 
	parslogentry = malloc(strlen(ptr) + 1);
	// copy @ position 1 (due to leading space) until strlen
	strncpy(parslogentry, ptr + 1, strlen(ptr));
	
	// make copy of logentry to tokenize again to get username
	logentrycopy = malloc(strlen(parslogentry + 1));	
	strcpy(logentrycopy, parslogentry);

	ptr2 = strtok(logentrycopy, " ");
	ptr2 = strtok(NULL, " ");
	ptr2 = strtok(NULL, " ");
	ptr2 = strtok(NULL, " ");
	parsusr = malloc(strlen(ptr2) + 1);
	strcpy(parsusr, ptr2);

	// time, log entry, user to pass to email

	msgitem[0] = parstime;
	msgitem[1] = parslogentry;
	msgitem[2] = parsusr;
}
	// if last line no match return to main function	
else {
	inotify_function();
}

fclose(f);
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

// check for event, specifically when auth.log file is modified

if (event->len > 0) {
	if (strcmp(event->name, "auth.log") == 0) {
		// then call log reader
		logfilereader();		
		// then email
		snprintf(embuff, sizeof embuff, "echo \"%s %s\" | mailx -s \"user login: %s\" %s", msgitem[0], msgitem[1], msgitem[2], emailaddy);
		system(embuff);
		
		// also write to our log
		logger(msgitem[1]); 
		
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

FILE* proglogfp;
proglogfp=fopen(proglog, "a");

if (proglogfp != NULL) {
	fprintf(proglogfp, "%s: %s\n", timebuf, message);
	fclose(proglogfp);
}
else {
	printf("ERROR: Unable to write to program log: %s\n", proglog);
	exit(1);
}

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

if (access("/usr/sbin/ssmtp", F_OK) == -1) {
        logger("Error: ssmtp not installed at: /usr/sbin/ssmtp");
        exit(1);
}

if (argc != 1) {
	printf("err\n");
}

else {
	// fork before calling main program
	forker();
}
}
