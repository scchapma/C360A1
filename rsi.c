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


/* Contents of file command-line argument */
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
	
	/* tokenize files command-line argument and store file strings in dynamic array */
   	
	int parseInput(char* input){
	    char *separator = " \t";
	    char *basic_token = strsep(&input, separator);
	    char *token;
    
	    while (basic_token != NULL){
		token = string_duplicator(basic_token);
	        addstring(token);
		basic_token = strsep(&input, separator);
	    }
	    addstring(NULL);	
	    return 0;
	}		

int reset_string_array(){
	stringtab.stringval = 0;
        stringtab.max = 0;
        stringtab.stringval = NULL;
        free(stringtab.stringval);
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


int main() {
	char* prompt = getPrompt();  
        int bailout = 1; 
	int status;
	
	while (bailout) {
		// Get user input
		char *reply = readline(prompt);
		/* if user quits, exit loop */
		if (!strcmp(reply, "quit")) {
			bailout = 0;
		} else { // Execute user command
		        
			// 1. Parse the user input contained in reply
			// (Hint: tokenize it by strtok, space as the delimiter)			
			//this function from c library for strsep()
			/*
			char **ap, *argv[10];
			for (ap = argv; (*ap = strsep(&reply, " \t")) != NULL;)
				if (**ap != '\0')
					//printf("Token: %s\n", *ap);
					if (++ap >= &argv[10])
						break;
			*/
			
			parseInput(reply);

			// 2. If "cd", then change directory by chdir()
			
			// 3. Else, execute command by fork() and exec()
			
			if(fork() == 0){
				//execvp(*argv, argv);
				execvp(*stringtab.stringval, stringtab.stringval);
				perror("Error on execvp");
				exit(1);
				/*if(execvp(argv[0], argv)<0){
					perror("Err on execvp");
					//need exit here?
					exit(1);
				}*/
				printf("Child process, post exec call.\n");	
		        }else{
				wait(&status);
			}
		}
		reset_string_array();
		free(reply);
	}
	printf("RSI:  Exiting normally.\n");
	free(prompt);
	return(0);
}


