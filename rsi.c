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
int concat(char* str1, char* str2, char* str3, char* out){
    if (!str1 || !str2 || !str3 || !out)
	return -1;

    while (*str1)
        *out++ = *str1++;
    while (*str2)
        *out++ = *str2++;
    while (*str3)
        *out++ = *str3++;
    *out = '\0';
    return 0;
} 


char* getPrompt(){
	// Write Code Here
    char* buf = NULL;
    size_t size = 0;
    char* path = getcwd(buf, size);
    printf("Path: %s\n", path);

    int path_size = strlen(path);
    printf("Path_size: %d\n", path_size);
    
    char header[] = "RSI: ";
    char footer [] = " > ";
    //char prompt2[strlen(header)*4 + strlen(path)*4 + strlen(footer)*4 + 4];
    
    //change this to emalloc() from S265
    //give credit to Zastre
    char* result = (char*) malloc(sizeof(char) * (path_size + 10)); 

    if (!path){
        printf("Error - could not find current working directory.\n");
        printf("Error:  %s\n", strerror(errno));
    }else{
        strncpy(result, header, strlen(header));
        strncat(result, path, strlen(path));
        strncat(result, footer, strlen(footer));
	printf("Prompt: %s\n", result);
        //char * reply = readline(result);
    }
    free(buf); 
    return result;
}

/*	main
*/
int main() {
	char* prompt = getPrompt();  
	printf("Prompt2: %s\n", prompt);
        int bailout = 1; 
	
	while (bailout) {
		// Get user input
	        char *reply = readline(prompt);
		/* if user quits, exit loop */
		if (!strcmp(reply, "quit")) {
			bailout = 0;
		} else { // Execute user command
		
			// 1. Parse the user input contained in reply
			// (Hint: tokenize it by strtok, space as the delimiter)
			
			// 2. If "cd", then change directory by chdir()
			
			// 3. Else, execute command by fork() and exec()
			char* argv_execvp[] = {"-l", "/usr/bin",  NULL};
			if(fork() == 0)
				if(execvp("ls", argv_execvp)<0)
					perror("Err on execvp");
		}
        	free(reply);
	}
	printf("RSI:  Exiting normally.\n");
	free(prompt);
	return(0);
}


