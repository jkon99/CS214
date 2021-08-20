CS 214: Systems Programming
Assignment 2: WTF
Authors:
	Jonathan Konopka
	NETID: jk1549
	RUID: 178005220
	Nicholas Rytelewski
	NETID: nr548
	RUID: 197000952

Project Description :
	This project acts as a very simplified version of github, where a server contains a
	collective version of a repository, and clients have their own version of the
	repository locally. In order to keep versions of the code the same amongst the
	group, a client can upload (push), and download (pull) each other’s versions of the
	code to and from the server. Our WTF system included the commands ‘configure’,
	‘checkout’, ‘update’, ‘upgrade’, ‘commit’, ‘push’, ‘create’, ‘destroy’, ‘add’,
	‘remove’, ‘currentversion’, ‘history’, and ‘rollback’, all of which were stated in the
	original assignment PDF.

Thread Synchronization :
	Our program made use of threads by calling a fork() command whenever a new
	client connection was accepted on the server side. Our fork() was assigned to a pid
	int and if the int equaled zero on the server (indicating that the process currently
	happening is a child of the server’s main process), then the process would carry on
	executing, and completing the requests sent by the server until the operation was
	over. The main process (PID != 0) would never be the one executing client
	requests.

Functions: listed in pdf