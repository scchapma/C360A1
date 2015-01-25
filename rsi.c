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
	//printf("Input = %s\n", *args);
        int size = 100;
        char cur[size];
        getcwd(cur,size);
        //printf("The current working directory of cur: %s\n", cur);

        //confirm length of array <= 2
	/*if(args[2] != NULL){
		printf("Format error.  Correct format is \"cd\" or \"cd arg1\".\n");
		return 0;
	}*/
	
	char* new_directory;
	//special cases: cd, cd ~, and cd ..
	//if (strcmp(args[1], "~") == 0){
	
	if (!args[1]) { 
		new_directory = getenv("HOME");
	} else if (strcmp(args[1], "~") == 0){
        	new_directory = getenv("HOME");
	} else if (args[2] != NULL){
                printf("Format error.  Correct format is \"cd\" or \"cd arg1\".\n");
                return 0;
        } else {
		new_directory =args[1];
	}
	
	//confirm length of array <= 2
        /*
	if(args[2] != NULL){
                printf("Format error.  Correct format is \"cd\" or \"cd arg1\".\n");
                return 0;
        }*/   
	
        //printf("Now, let's change the working directory.\n");
        if (chdir(new_directory)==0){  // Success
         	getcwd(cur,size);
               	//printf("The current working directory of cur: %s\n", cur);
   	}else{   //Failure
              	getcwd(cur,size);
		printf("Error - could not locate directory \"%s\".\n", args[1]);
             	//printf("The current working directory of cur: %s\n", cur);
      	}		
}

int reset_string_array(){
        stringtab.stringval = 0;
        stringtab.max = 0;
        stringtab.stringval = NULL;
        free(stringtab.stringval);
        return 0;
}


int main() {
	//char* prompt = getPrompt();  
        int bailout = 1; 
	int status;
	
	while (bailout) {
		// Get user input
		char* prompt = getPrompt();
		char *reply = readline(prompt);
		/* if user quits, exit loop */
		if (!strcmp(reply, "quit")) {
			bailout = 0;
		} else { // Execute user command
		        
			// 1. Parse the user input contained in reply
			parseInput(reply);

			// 2. If "cd", then change directory by chdir()
			if (strcmp(*stringtab.stringval, "cd") == 0){
				parse_cd(stringtab.stringval);
			} else {
			
				// 3. Else, execute command by fork() and exec()
				if (fork() == 0){
					execvp(*stringtab.stringval, stringtab.stringval);
					perror("Error on execvp");
					exit(1);
					printf("Child process, post exec call.\n");	
		        	} else {
					wait(&status);
				}
			}
		}
		reset_string_array();
		free(reply);
		free(prompt);
	}
	printf("RSI:  Exiting normally.\n");
	//free(prompt);
	return(0);
}


