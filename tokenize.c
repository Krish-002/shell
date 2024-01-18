#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_STRING 255

// enums for to see if it is "" or not
enum State {
    DEFAULT,
    QUOTE
};


// actual function
void tokenize(char *str) {
    int state = DEFAULT;
    char *token = malloc(MAX_STRING * sizeof(char));
    int j = 0;

    // goes through the input
    for (int i = 0; i < strlen(str); i++) {
       // handles cases with double quotes and normal
	    switch (state) {
            case DEFAULT:
		    // handle tabs and newlines
                if (str[i] == ' ' || str[i] == '\t' || str[i] == '\n') {
                    if (j > 0) {
			    // end it
                        token[j] = '\0';
                        printf("%s\n", token);
                        j = 0;
                    }
		    // change state if it is
                } else if (str[i] == '"' || str[i] == '\'') {
                    state = QUOTE;
		    // handle cases given in the axmaples
                } else if (str[i] == '(' || str[i] == ')' || str[i] == '<' || str[i] == '>' || str[i] == ';' || str[i] == '|') {
                    printf("%c\n", str[i]);
                } else {
                    token[j++] = str[i];
                }
                break;
            case QUOTE:
		// double quote case
                if (str[i] == '"' || str[i] == '\'') {
                    state = DEFAULT;
                    if (j > 0) {
                        token[j] = '\0';
                        printf("%s\n", token);
                        j = 0;
                    }
                } else {
                    token[j++] = str[i];
                }
                break;
        }
    }

    if (j > 0) {
        token[j] = '\0';
        printf("%s\n", token);
    }

    free(token);
}

int main(int argc, char **argv) {
    char input[MAX_STRING + 1];
    if (!fgets(input, sizeof(input), stdin)) {
        fprintf(stderr, "Error reading input\n");
        return 1;
    }

    // Remove newline character from input, if it exists
    size_t len = strlen(input);
    if (len > 0 && input[len - 1] == '\n') {
        input[len - 1] = '\0';
    }

    tokenize(input);

    return 0;
}
