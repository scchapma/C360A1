#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <signal.h>

typedef int bool;
#define true 1
#define false 0

/* Contents of user input string */
struct Stringtab{
    int sval;
    int max;
    char **stringval;
} stringtab;

enum {FINIT = 1, FGROW = 2};


/* -------------------------------------------------------------------------------------------
** 		ACCESSORY FUNCTIONS
** -------------------------------------------------------------------------------------------
*/

/* Dr. Zastre's code from class */
char *string_duplicator(char *input) {
    char *copy;
    assert (input != NULL);
    copy = (char *)malloc(sizeof(char) * strlen(input) + 1);
    if (copy == NULL) {
        fprintf(stderr, "error in string_duplicator");
        exit(1);
    }
    strncpy(copy, input, strlen(input)+1);
    return copy;
}

/* Dr. Zastre's code from SENG 265 */
void *emalloc(size_t n){
    void *p;
    p = malloc(n);
    if (p == NULL) {
        fprintf(stderr, "malloc of %lu bytes failed", n);
        exit(1);
    }
    return p;
}

/* Amended version of Dr. Zastre's "addname" code from SENG 265 */
int addstring(char *newstring){
	char **fp;
    
	if(stringtab.stringval == NULL){
	    	stringtab.stringval = (char **) emalloc(FINIT*sizeof(char *));
	        stringtab.max = FINIT;
	        stringtab.sval = 0;
	}else if(stringtab.sval >= stringtab.max){
	        fp = (char **) realloc(stringtab.stringval, (FGROW*stringtab.max)*sizeof(char *));
	        if(stringtab.stringval == NULL){
	            	return -1;
	        }
	        stringtab.max = FGROW*stringtab.max;
	        stringtab.stringval = fp;
	}
	stringtab.stringval[stringtab.sval] = newstring;
	return stringtab.sval++;
	}

/* -------------------------------------------------------------------------------------------
**              MAIN FUNCTIONS
** -------------------------------------------------------------------------------------------
*/
	
/* tokenize files command-line argument and store file strings in dynamic array */
   	
int parseInput(char* input, bool* background_p){
	char *separator = " \t";
	char *basic_token = strsep(&input, separator);
	char *token;
    
	while (basic_token != NULL){
		token = string_duplicator(basic_token);
	        addstring(token);
		basic_token = strsep(&input, separator);
	}
	//check for ampersand
	//printf("Check for &.\n");
	//printf("sval: %d.\n", stringtab.sval);
	//printf("a[sval]: %s\n", stringtab.stringval[stringtab.sval-1]);
	//if time, change to ternary operator
	if (strcmp(stringtab.stringval[stringtab.sval-1], "&") == 0){
		stringtab.stringval[stringtab.sval-1] = NULL;
		*background_p = true;
	} else {
		*background_p = false;
	}
	//printf("post strcmp.\n");
	addstring(NULL);	
	return 0;
}		

char* getPrompt(){
    char* buf = NULL;
    size_t size = 0;

    char* path = getcwd(buf, size);
    int path_size = strlen(path);
    
    char header[] = "RSI: ";
    char footer [] = " > ";
    char* result = (char*) emalloc(sizeof(char) * (path_size + 10)); 

    if (!path){
        printf("Error - could not find current working directory.\n");
        printf("Error:  %s\n", strerror(errno));
    }else{
        strncpy(result, header, strlen(header));
        strncat(result, path, strlen(path));
        strncat(result, footer, strlen(footer));
    }
    free(buf); 
    return result;
}


int parse_cd (char** args){

	char* new_directory = NULL;

	//special case: cd (no argument)	
	if (!args[1]) { 
		new_directory = getenv("HOME");
	//confirm length of array <= 2
	} else if (args[2] != NULL){
                printf("Format error.  Correct format is \"cd\" or \"cd arg1\".\n");
                return 0;
        //special case: cd ~
	} else if (strcmp(args[1], "~") == 0){
                new_directory = getenv("HOME");
	} else {
		new_directory =args[1];
	}
	
        if (chdir(new_directory)==0){  // Success
         	return 0;
   	}else{   //Failure
		printf("Error - could not locate directory \"%s\".\n", args[1]);
             	return 1;
      	}	
}

int parse_pwd (){
	char* directory = NULL;
	int size = 100;
        char cur[size];
        
	directory = getcwd(cur,size);
	printf("%s\n", directory);
	return 0;
	
}

int reset_string_array(){
        stringtab.stringval = 0;
        stringtab.max = 0;
        stringtab.stringval = NULL;
        free(stringtab.stringval);
        return 0;
}

int execute_command(char* argc, char** args, bool* in_background){
	pid_t cpid, w;
	int status;

	//printf("Execute_command in_background: %d\n", *in_background);

	cpid = fork();
	if (cpid == -1) {
		perror("fork");
		exit(EXIT_FAILURE);
	}

	if (cpid == 0){
        	//printf("Child PID is %ld\n", (long) getpid());
		execvp(*stringtab.stringval, stringtab.stringval);
        	perror("Error on execvp");
       		exit(1);
                printf("Child process, post exec call.\n");
        } else {
             	//if in background
		if (*in_background){
			/*do {
                   		//w = waitpid(cpid, &status, WUNTRACED | WCONTINUED);
                   		w = waitpid(cpid, &status, WNOHANG);
				if (w == -1) {
                       			perror("waitpid");
                       			exit(EXIT_FAILURE);
                   		}

                   		if (WIFEXITED(status)) {
                       			printf("exited, status=%d\n", WEXITSTATUS(status));
                   		} else if (WIFSIGNALED(status)) {
                       			printf("killed by signal %d\n", WTERMSIG(status));
                   		} else if (WIFSTOPPED(status)) {
                       			printf("stopped by signal %d\n", WSTOPSIG(status));
                   		} else if (WIFCONTINUED(status)) {
                       			printf("continued\n");
                   		}
               		//} while (!WIFEXITED(status) && !WIFSIGNALED(status));
			} while (!WIFEXITED(status) && !WIFSIGNALED(status));*/
				//printf("Running in background.\n");
				//waitpid(cpid, &status, WNOHANG);
				wait(&status);
				//exit(0);
		}else {
			//printf("Not running in background.\n");
			wait(&status);
			//exit(0);
		}
       	}
	return 0;
}


int main() {
	  
        int bailout = 1; 
	int status;
	
	while (bailout) {
		/* Get user input */
		char* prompt = getPrompt();
		char* reply = readline(prompt);
		
		/* if user quits, exit loop */
		if (!strcmp(reply, "quit")) {
			bailout = 0;
		
		/* otherwise, execute user command*/
		} else { 
		        
			// 1. Parse the user input contained in reply
			bool in_background;; 
			parseInput(reply, &in_background);	
			
			if (!*stringtab.stringval) {
				printf("Error: Null array or singleton '&'.\n");
			
			// 2. If "cd", then change directory by chdir()
			}else if (strcmp(*stringtab.stringval, "cd") == 0){
				parse_cd(stringtab.stringval);
			// 3. if "pwd", return the current directory
			} else if (strcmp(*stringtab.stringval, "pwd") == 0) {
				parse_pwd();
			} else {
			
				// 4. Else, execute command by fork() and exec()
				execute_command(*stringtab.stringval, stringtab.stringval, &in_background);
			}
		}
		reset_string_array();
		free(reply);
		free(prompt);
	}
	printf("RSI:  Exiting normally.\n");
	return(0);
}


