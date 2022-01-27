#pragma once
#include <vector>
#include <stdio.h>
#include <cstdlib>
#include <iostream>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>

#define MAX_LINE 80

using namespace std;

class simple_shell {
public:
	simple_shell();

	bool shouldRun;
	bool shouldWait;
	void saveCommand(char command[]);
	void tokenize(char command[], char * args[]);	 //turn command into a list of args[]
	void execute(char * args[]);						//execute command based on args list
	void displayHistory();

private:
	vector<string> commandHistory;		/*	was using char* to store commands but it is not a good paradigim, check out this stackoverflow answer */
										/* https://stackoverflow.com/questions/7555820/how-to-work-with-null-pointers-in-a-stdvector */
};