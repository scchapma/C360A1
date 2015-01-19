#include <stdio.h>
#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <signal.h>

/*	char* getPrompt()
	Eetrieve current working directory (see getcwd())
	if successful, returns complete prompt with absoluate path = "RSI: [cwd] >"
*/
char* getPrompt(){
	// Write Code Here
    // return char*
    char* buf;
    size_t size;
    char* path = getcwd(NULL, size);
    if (!path){
        //print error message from errno
        printf("Error - could not find current working directory.\n");
    }else{
        printf("RSI: %s\n", path);
    }
    free(buf);
    return path;
}

/*	main
*/
int main() {
	char* prompt = getPrompt();  //
	int bailout = 1;
	
	while (bailout) {
		// Get user input
		char* reply = readline(prompt);
		//char* reply = "Hello, World!";
		/* if user quits, exit loop */
		if (!strcmp(reply, "quit")) {
			bailout = 0;
		} else { // Execute user command
		
			// 1. Parse the user input contained in reply
			// (Hint: tokenize it by strtok, space as the delimiter)
			
			// 2. If "cd", then change directory by chdir()
			
			// 3. Else, execute command by fork() and exec()
		}
        //free(reply)
	}
	printf("RSI:  Exiting normally.\n");
	free(prompt);
	return(0);
}
