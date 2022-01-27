#include <iostream>
#include "simple_shell.h"


int main(void) {
	cout << "~~~~~     Danny Ha, Jake Tremblay, Brian Richey         ~~~~~~" << endl;
	cout << "~~~~~ Project 2: simple-shell using fork() and execvp() ~~~~~~" << endl;
	cout << "~~~~~                   Fall 2021                       ~~~~~~" << endl;


	simple_shell osh; //sets shouldRun to true on creation

	/*
	* After reading user input, steps are: 
	* 1- fork a child process using fork()
	* 2- the child process will invoke execvp()
	* 3- parent will invoke wait() UNLESS the command included &
	*/
	while (osh.shouldRun) {						/*!DONT FORGET TO SET OSH.SHOULDRUN TO FALSE, EVENTUALLY!*/
		char commands[MAX_LINE];
		char * args[MAX_LINE / 2 + 1];

		osh.shouldWait = false;

		cout << "osh> ";
		cout.flush();

		cin.getline(commands, MAX_LINE);

		osh.saveCommand(commands);

		osh.tokenize(commands, args);	//this constructs args[] for execvp

		osh.execute(args); //this calls fork and execvp

	}

	return 0;
}