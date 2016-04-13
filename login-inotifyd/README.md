# login-inotifyd (0.2)

## Synopsis

login-inotifyd - A daemon that detects SSH logins and e-mails notifications using mailx and SSMPT. Root access is required. This utilizes the inotify API and e-mails notifications immediately without the need for a cron job. 

Previous versions in the 'old' subfolder do not use inotify. 

## Installation

1. Prior to installation mailx and SSMTP must be installed. 

2. Edit login-inotifyd.c file and update e-mail address.

3. $ make login-inotifyd
	- place executable in location of your choice. ie: /usr/bin/login-inotifyd

4. Update included /etc/login-inotifyd.service file and change ExecStart line to reflect location of executable.
	- move service file to /etc/systemd/system/ and activate:
	- $ systemctl enable login-inotifyd.service
	- $ systemctl start login-inotifyd.service

	OR:

	- If using SysVinit, install included /etc/login-inotifyd file in /etc/init.d/ and configure as necessary. 
	- $ update-rc.d login-inotifyd defaults
	- $ service login-inotifyd start

## Updates
April 2016: 
-Now reading from /var/log/auth.log rather than /var/log/wtmp to allow notifications from SFTP logins.
-Prevention of programs like xterm or tmux sending a notification; user name now in e-mail subject line; function prototypes added.

## License
Copyright 2015-16 Matthew Wilson. 
License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>.
No warranty. Software provided as is.
