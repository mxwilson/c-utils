# login-inotifyd (0.1.7)

## Synopsis

login-inotifyd - A daemon that detects SSH logins and e-mails notifications using mailx. An MTA such as Postfix or SSMPT must be installed to send
the mail. Root access is required. This program utilizes the inotify API and e-mails notifications immediately without the need for a cron job.

Previous versions in the 'old' subfolder do not use inotify. 

## Installation

1. Prior to installation mailx and an MTA must be installed. 

2. Edit login-inotifyd.c file and add your e-mail address.

3. $ make login-inotifyd
	- place executable in location of your choice. ie: /usr/bin/login-inotifyd

4. Update included /etc/login-inotifyd.service file and change ExecStart line to reflect location of executable.
	- move service file to /usr/lib/systemd/system/ and activate:
	- $ systemctl enable login-inotifyd.service
	- $ systemctl start login-inotifyd.service

	OR:

	- If using SysVinit, install included /etc/login-inotifyd file in /etc/init.d/ and configure as necessary. 
	- $ update-rc.d login-inotifyd defaults
	- $ service login-inotifyd start

## Updates
Nov 2016: Code cleanup.
July 2016: Program is now MTA agnostic.
April 2016: Prevention of programs like xterm or tmux sending a notification; user name now in e-mail subject line; function prototypes added.

## License
Copyright 2015-16 Matthew Wilson. 
License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>.
No warranty. Software provided as is.
