#define _POSIX_C_SOURCE 200809L

// all relevant imports
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define MAX_STRING 255
#define MAX_ARGS 20


// Enums for tokenize function
enum State {
    DEFAULT,
    QUOTE
};

// declaring placeholder,  used in the prev command implementation
char prev_command[MAX_STRING + 1] = "";


// tokenize funtion (part 1)
char** tokenize(char *str) {
    int state = DEFAULT;
    char *token = malloc(MAX_STRING * sizeof(char));
    char **tokens = malloc(MAX_ARGS * sizeof(char*));
    int j = 0;
    int k = 0;

    //loops through the input of the user, to seperate everything into tokens 
    for (int i = 0; i < strlen(str); i++) {
	    // check if input is in  "" or not, and process accordingly:
        switch (state) {
            case DEFAULT:
		    //for if tehre is a tab or new line
                if (str[i] == ' ' || str[i] == '\t' || str[i] == '\n') {
			// end it
                    if (j > 0) {
                        token[j] = '\0';
                        tokens[k++] = strdup(token);
                        j = 0;
                    }
                } else if (str[i] == '"') {
                    state = QUOTE;
                } else {
                    token[j++] = str[i];
                }
                break;
	
            case QUOTE:
                if (str[i] == '"') {
                    state = DEFAULT;
                } else {
                    token[j++] = str[i];
                }
                break;
        }
    }

    if (j > 0) {
        token[j] = '\0';
        tokens[k++] = strdup(token);
    }
    tokens[k] = NULL;
    free(token);
    return tokens;
}

// helper function for 'help' cmd. 
void displayHelp() {
    printf("Built-in commands:\n");
    printf("cd [directory] : Change directory.\n");
    printf("prev : Print and execute previous command.\n");
    printf("source [filename] : Execute script from a file.\n");
    printf("help : Display help (ur alr here).\n");
    printf("exit : Exit shell.\n");
}


// helper for the source cmd which excutes a script (because not in-built system cmd)
void executeSource(const char *filename) {
    FILE *fp = fopen(filename, "r");
    if (fp) {
        char line[MAX_STRING + 1];
        while (fgets(line, sizeof(line), fp) != NULL) {
            system(line);
        }
        fclose(fp);
    } else {
        perror("shell");
    }
}

// helper for redirection pipes and sequencing
void handleExecution(char **args) {
    int in = 0, out = 1;
    char *inputFile = NULL, *outputFile = NULL;

    // Check for input redirection
    for (int i = 0; args[i] != NULL; i++) {
        if (strcmp(args[i], "<") == 0) {
		// ignore the symbol < and carry out the process
            args[i] = NULL;  
            inputFile = args[i + 1];
            in = open(inputFile, O_RDONLY);
            if (in < 0) {
                perror("shell");
                return;
            }
        }
    }

    // Check for output redirection
    for (int i = 0; args[i] != NULL; i++) {
        if (strcmp(args[i], ">") == 0) {
		// similar to input redirect
            args[i] = NULL; 
            outputFile = args[i + 1];
            out = open(outputFile, O_WRONLY | O_CREAT | O_TRUNC, 0666);
            if (out < 0) {
                perror("shell");
                return;
            }
        }
    }

    // Check for piping
    // split the two parts of the pipe in an array
    int pipes[2];
    // contains the cmd in the second part of teh pipe
    char **command2 = NULL;

    // look for " | " and terminate command 1 and set command2 as the second command
    // craete a pipe
    for (int i = 0; args[i] != NULL; i++) {
        if (strcmp(args[i], "|") == 0) {
            args[i] = NULL;
            command2 = &args[i + 1];
            pipe(pipes);
        }
    }

    // creat a child process
    // if there is input redirection update std input to read from in
    // if there is output redirect, stdout to write to out
    // if second command exists,, std out to write to the write of the pipe
    // excute first cmd
    pid_t pid = fork();
    if (pid == 0) {  
        if (in != 0) {
            dup2(in, 0);
            close(in);
        }
        if (out != 1) {
            dup2(out, 1);
            close(out);
        }
        if (command2) {
            dup2(pipes[1], 1);
            close(pipes[0]);
            close(pipes[1]);
        }
        if (execvp(args[0], args) == -1) {
            printf("%s: command not found\n", args[0]);
            exit(EXIT_FAILURE);
        }
    }

    // Handle the second command in the pipe, if there is one
    if (command2) {
        pid_t pid2 = fork();
        if (pid2 == 0) {
            dup2(pipes[0], 0);
            close(pipes[0]);
            close(pipes[1]);
            if (execvp(command2[0], command2) == -1) {
                printf("%s: command not found\n", command2[0]);
                exit(EXIT_FAILURE);
            }
        }
        close(pipes[0]);
        close(pipes[1]);
        wait(NULL);
    }

    wait(NULL);
}


// main function
int main(int argc, char **argv) {
	//welcome user
    printf("Welcome to mini-shell.\n");
    
    // keep track of the previous cmd
    char prev_command[MAX_STRING + 1] = "";

    // infite loop until exited
    while (1) {
        char input[MAX_STRING + 1];
        printf("shell $ ");
	// if failed then exit
        if (!fgets(input, sizeof(input), stdin)) {
            printf("Bye bye.\n");
            break;
        }

	// remove newlines
        size_t len = strlen(input);
        if (len > 0 && input[len - 1] == '\n') {
            input[len - 1] = '\0';
        }

	// check is user enetered prev
        if (strcmp(input, "prev") == 0) {
            printf("%s\n", prev_command);
            strncpy(input, prev_command, MAX_STRING);
        }

        // Split commands by ';'
        char *command = strtok(input, ";");

	//loop through teh cmds
        while (command != NULL) {

	// call the helper tokenize
            char **args = tokenize(command);

            if (args[0] == NULL) {
                free(args);
                command = strtok(NULL, ";");
                continue;
            }

	    // handle "exit"
            if (strcmp(args[0], "exit") == 0) {
                printf("Bye bye.\n");
                free(args);
                exit(0);
            } 
	    
	    // handle "help"
	    else if (strcmp(args[0], "help") == 0) {
                displayHelp();
            }

	   // handle "source"
	    else if (strcmp(args[0], "source") == 0) {
                if (args[1] != NULL) {
                    executeSource(args[1]);
                } else {
                    printf("source: Please provide a script file to execute.\n");
                }
            } 

	    // handle cd
	    else if (strcmp(args[0], "cd") == 0) {
                if (args[1] != NULL) {
                    chdir(args[1]);
                } else {
                    chdir(getenv("HOME"));
                }
            }
	   
	   // handle command from part 2.2 to 2.5 (redirection sequencing, and pipes)
	    else {
                handleExecution(args);
            }

	    // free alllocated memory

            for (int i = 0; args[i] != NULL; i++) {
                free(args[i]);
            }
            free(args);
            command = strtok(NULL, ";");
        }
        strncpy(prev_command, input, MAX_STRING);
    }
    return 0;
}

