## Synopsis

Login-notifyd - A simple Systemd daemon that detects SSH logins and e-mails notifications using mailx and SSMPT. Root access is required. 

## Installation

1. Edit login-notifyd.c file and update e-mail address and log file variables.

2. $ make login-notifyd
	> then place executable in location of your choice - ie - /usr/local/bin/login-notifyd

3. Update login-notifyd.service file and change ExecStart line to reflect location of executable.
	> move service file to /etc/systemd/system/ and activate:
		> $ systemctl enable login-notifyd.service
		> $ systemctl start login-notifyd.service

## License
Copyright 2015 Matthew Wilson. 
License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>.
No warranty. Software provided as is.
