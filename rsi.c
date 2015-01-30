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

/* Node struct for list of background functions */
typedef struct BG_Job {
	pid_t pid;
	char *name;
	int status;
	struct BG_Job *next;
} BG_Job;

/* Head for list of background functions */
BG_Job *bg_list = NULL;


/* -------------------------------------------------------------------------------------------
** 		ACCESSORY FUNCTIONS
** -------------------------------------------------------------------------------------------
*/

/* Dr. Zastre's code from SENG 265 */
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
**              LINKED LIST FUNCTIONS
** -------------------------------------------------------------------------------------------
*/

/* Node constructor */
BG_Job *newitem (pid_t pid, char *name, char status){
	BG_Job *newp;
	
	newp = (BG_Job *) emalloc(sizeof(BG_Job));
	newp->pid = pid;
	newp->name = name;
	newp->status = status;
	newp->next = NULL;
	return newp;
}

/* Add new node to front of list */
BG_Job *addfront (BG_Job *listp, BG_Job *newp){
	newp->next = listp;
	return newp;
}


/* Delete item at given pointer */
BG_Job *delitem (BG_Job *listp, BG_Job *targetp){
        BG_Job *p, *prev;

        prev = NULL;
        for (p = listp; p != NULL; p = p-> next){
                if (p == targetp){
                        if (prev == NULL){
                                listp = p->next;
                        }else{
                                prev->next = p->next;
                        }
                        free(p);
                        return listp;
                }
                prev = p;
        }
        fprintf(stderr, "delitem: %s not in list", targetp->name);
        exit(1);
}

/* Free memory for all remaining nodes in list */
void freeall (BG_Job *listp) {
	BG_Job *next;

	for ( ; listp != NULL; listp = next){
		next = listp->next;
		free(listp);
	}
}

/* Traverse list and delete nodes that have terminated*/
void check_bg_list(BG_Job *listp){
	
	for ( ; listp != NULL; listp = listp->next){
		int retVal = waitpid(listp->pid, &listp->status, WNOHANG);
		if (retVal == -1){
			perror("waitpid"); 
			exit(EXIT_FAILURE);
		}
		if(retVal > 0){	
			printf("Background process terminated - pid: %d, command: %s.\n", 
				listp->pid, listp->name, WEXITSTATUS(listp->status));
			bg_list = delitem(bg_list, listp);
		}
	}
}


/* -------------------------------------------------------------------------------------------
**              MAIN FUNCTIONS
** -------------------------------------------------------------------------------------------
*/
	
/* tokenize files command-line argument and store file strings in dynamic array */
int parseInput(char* input, bool* background_p){

	char *separator = " ";
	char *basic_token = strtok(input, separator);
	char *token;

	while (basic_token != NULL){
		token = string_duplicator(basic_token);
	        addstring(token);
		basic_token = strtok(NULL, separator);
	}

	if(stringtab.sval == 0){
		return 0;
	}
	
	//check for ampersand
	if (strcmp(stringtab.stringval[stringtab.sval-1], "&") == 0){
		stringtab.stringval[stringtab.sval-1] = NULL;
		*background_p = true;
	} else {
		*background_p = false;
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

int parse_pwd (char** args){
	char* directory = NULL;
	int size = 100;
        char cur[size];

	//confirm length of array == 1
        if (args[1] != NULL){
                printf("Format error.  Correct format is \"pwd\" with no additional arguments.\n");
                return 0;
        }

	directory = getcwd(cur,size);
	printf("%s\n", directory);
	return 0;
	
}

int reset_string_array(){
        stringtab.sval = 0;
        stringtab.max = 0;
        stringtab.stringval = NULL;
        free(stringtab.stringval);
        return 0;
}

int execute_command(char* argc, char** args, bool* in_background){
	pid_t cpid;
	int status;

	cpid = fork();
	if (cpid == -1) {
		perror("fork");
		exit(EXIT_FAILURE);
	}

	if (cpid == 0){ 
        	if (execvp(argc, args) == -1){
			printf("%s: Command not found.\n", argc);
		}
		//perror("Command not found.\n");
		exit(1);
        } else {
		if (*in_background){
			//if in background, add node to background_list
			printf("Process running in background - pid: %d.\n", cpid);
			bg_list = addfront(bg_list, newitem(cpid, argc, status));
		}else {	
			waitpid(cpid, &status, 0);
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

		/* Check if string length is 0 */
		if ((int)strlen(reply) == 0){
			continue;
		}

		/* Traverse in_background linked list */
		check_bg_list(bg_list);	
	
		/* if user quits, exit loop */
		if (!strcmp(reply, "quit")) {
			bailout = 0;
		
		/* otherwise, execute user command*/
		} else { 
		        
			//parse the user input contained in reply
			bool in_background;; 	
			parseInput(reply, &in_background);	
			
			if (stringtab.sval == 0) { 
                                continue;

			} else if (!*stringtab.stringval) {
				printf("Error: Null array or singleton '&'.\n");

			//if "cd", then change directory by chdir()
			} else if (strcmp(*stringtab.stringval, "cd") == 0){
				parse_cd(stringtab.stringval);
			
			//if "pwd", return the current directory
			} else if (strcmp(*stringtab.stringval, "pwd") == 0) {
				parse_pwd(stringtab.stringval);
			
			//else, execute command by fork() and exec()
			} else {
				execute_command(*stringtab.stringval, stringtab.stringval, &in_background);
			}
		}
		reset_string_array();
		free(reply);
		free(prompt);
	}
	freeall(bg_list);
	printf("RSI:  Exiting normally.\n");
	return(0);
}


