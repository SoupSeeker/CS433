#include "simple_shell.h"

//basic constructor, sets shouldRun to true and shouldWait to false
simple_shell::simple_shell() {
	shouldRun = true;
	shouldWait = false;
}

//commandHistory is a vector of <string> commands that are obtained from doing strcpy on the getline() function
//more info about commandHistory in simple_shell.h
void simple_shell::saveCommand(char command[]) {
	string savedCommand(command);

	if (savedCommand != "!!" && savedCommand != "exit") {
		commandHistory.push_back(savedCommand);
	}
}

//extra functionality for debugging / development
void simple_shell::displayHistory() {
	for (int i = 0; i < commandHistory.size(); i++) {
		cout << "Command history: " << i << " " << commandHistory[i] << endl;
	}
}

//moved all of the command saving to the saveCommand function, tokenize is now soley for tokenizing, which is nice
void simple_shell::tokenize(char command[], char* args[]) {
	int i = 0;
	char* token = strtok(command, " ");

	while (token != NULL) {
		args[i] = token;
		i = i + 1;
		token = strtok(NULL, " ");
	}
	args[i] = NULL;
}

//execute handles many tasks, will scan the input for "exit", "!!", "<", ">", and "&" (in progress: "|")
void simple_shell::execute(char* args[]) {
	string str_args(args[0]);

	if (str_args == "exit") {
		shouldRun = false;
		return;
	}

	/*history functionality*/
	if (str_args == "!!") {
		if (!commandHistory.empty()) {
			int historySize = commandHistory.size();
			char last_command[MAX_LINE];

			strcpy(last_command, commandHistory[--historySize].c_str());
			
			commandHistory.pop_back();

			tokenize(last_command, args);
		}
		else {
			cout << "No commands in history!" << endl;
			return;
		}
	}

	//variables for file redirection 
	int fd;
	bool redir_out = false;
	bool redir_in = false;

	for (int i = 0; args[i] != NULL; i++) {				//go through args to find special characters
		if (strcmp(args[i], "<") == 0) {				//detect "<", peel off the "<" char and filename, set redir_in true
			redir_in = true;

			args[i] = NULL;
			args[i + 1] = NULL;

			break;
		}
		else if (strcmp(args[i], ">") == 0) {			//detect ">", peel off the ">" char and filename, set redir_out true
			redir_out = true;
							
			args[i] = NULL;
			args[i + 1] = NULL;
	
			break;
		}
		else if (strcmp(args[i], "&") == 0) {			//finds & and peels it off, sets shouldWait flag true
			args[i-1] = NULL;				
			shouldWait = true;
			break;
		}
	}

	pid_t pid = fork();						//fork the parent process


	if (pid == 0) {							//child process will do execvp.
		if (redir_out) {					
			fd = open("out.txt", O_WRONLY | O_CREAT, 0600);	

			dup2(fd, 1);

			close(fd);
		}
		else if (redir_in) {
			fd = open("in.txt", O_RDONLY | O_CREAT, 0600);

			dup2(fd, 0);

			close(fd);
		}
		
		execvp(args[0], args);

		exit(1);						//this will only get hit if there is an issue with a command
	}
	else if (pid > 0) {		
	
		if (shouldWait == false) {	//parent process will wait UNLESS there is &
			wait(0);
		}		
	}
	else {
		cout << "ERROR FORKING" << endl;
		shouldRun = false;
	}
}