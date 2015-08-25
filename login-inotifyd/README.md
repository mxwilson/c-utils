## Synopsis

Login-inotifyd - A daemon that detects SSH logins and e-mails notifications using mailx and SSMPT. Root access is required. Similar to prev programs except this utilizes inotify and e-mails notifications immediately without the need for a cron job. 

## Installation

1. Prior to installation mailx and SSMTP must be installed. 

2. Edit login-inotifyd.c file and update e-mail address.

3. $ make login-inotifyd
	- place executable in location of your choice. ie: /usr/bin/login-inotifyd

4. Update login-inotifyd.service file and change ExecStart line to reflect location of executable.
	- move service file to /etc/systemd/system/ and activate:
	- $ systemctl enable login-inotifyd.service
	- $ systemctl start login-inotifyd.service

	OR:

	- If using Upstart, install login-inotifyd.conf file in /etc/init/ and configure as necessary. 

## License
Copyright 2015 Matthew Wilson. 
License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>.
No warranty. Software provided as is.
