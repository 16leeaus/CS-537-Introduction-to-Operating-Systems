#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/wait.h>
#define Interactive 0
#define Batch 1
#define SIZE 512

/**
 * @brief Input/Output redirection function
 * 
 * @param args Argument used to redirect the stdin / stdout.
 * @return int Return 0 or 1 based on operation success.
 */
int check_redirection_format(char **args){
    if(strcmp(args[0], ">") == 0){
        write(2, "Redirection misformatted.\n", 26);
        return 0;
    }

    int c = 0;
    for(int i=0; args[i] != NULL; i++){
        if(strcmp(args[i], ">") == 0){
            c++;
            if(args[i+1] == NULL){
                write(2, "Redirection misformatted.\n", 26);
                return 0;
            }
        }
    }
    if(c > 1){
        write(2, "Redirection misformatted.\n", 26);
        return 0;
    }
    return 1;
}

/**
 * @brief Function used to parse the given line and extract the commands that 
 * are given to the shell program.
 * 
 * @param line Take in a char array, called line, given to the program by either
 * bathc mode and reading in a file. Or by using interactive mode and having the 
 * user provide input using stdin.
 * @return char** Return the command found by the parsing function, and send it 
 * to be executed by the launch function.
 */
char **getcmd(char *line){
    char cmd_orig[SIZE];
    char cmd[SIZE];
    char **parsedArgs;
    int i = 0;
    int k = 0;

    if(line != NULL){
        strcpy(cmd_orig, line);
        cmd_orig[strlen(cmd_orig)-1] = 32;

        for(int j=0; j<strlen(cmd_orig); j++){
            if(cmd_orig[j] == '>'){
                cmd[k] = ' ';
                k++;
                cmd[k] = '>';
                k++;
                cmd[k] = ' ';
                k++;
                continue;
            }
            cmd[k] = cmd_orig[j];
            k++;
        }

        parsedArgs = calloc(1, sizeof(char *));
        char *token = strtok(cmd, " ");

        while(token != NULL){
            parsedArgs[i] = token;
            ++i;
            parsedArgs = realloc(parsedArgs, sizeof(char*) * (i+1));
            token = strtok(NULL, " \n\r\t\f");

        }
        if(strcmp(parsedArgs[0], "exit") == 0){
            exit(0);
        }
        parsedArgs[i-1] = NULL;
    }
    else{
        exit(0);
    }
    return parsedArgs;
}

/**
 * @brief Function used to execute commands that have been parsed from either
 * interactive mode's stdin, or from reading a batch file in line by line.
 * 
 * @param path Path used to execute the given command.
 * @param args Command to execute, given by either the batch file or user input,
 * then parsed by the getcmd() function.
 * @return int Return status code based on operation success.
 */
int launch(char *path, char **args){
    if(args[0] == NULL){
        exit(0);
    }
    if(strcmp(path, "exit") == 0 || strcmp(args[0], "exit") == 0){
        exit(0);
    }

    int status;
    pid_t pid = fork();
    if(pid < 0){
        perror("Error Forking: ");
        _exit(pid);
    }
    if(pid == 0){
        //Child process
        for(int i=0; args[i] != NULL; i++){
            if(strcmp(args[i], ">") == 0){
                if(check_redirection_format(args) == 0)_exit(0);
                fclose(stdout);

                if(fopen(args[i+1], "w") == NULL){
                    write(2, "Cannot write to file foo.txt.\n", 31);
                    return 0;
                }
                args[i] = NULL;
                break;
            }
        }

        if(execv(path, args) == -1){
            write(2, args[0], strlen(args[0]));
            write(2, ": Command not found.\n", 22);
            _exit(pid);
            return 0;
        }
    }
    else{
        //Parent process
        waitpid(pid, &status, 0);
    }
    return 1;
}

/**
 * @brief Interactive mode: When no batch file is provided by the user,
 * interactive mode is initiated. Here, the shell will provide the user with a 
 * prompt on a loop. The user will then have the ability to provide commands 
 * using the standard input
 */
void interactive(){
    while(1){
        
        char line[SIZE];
        write(1, "mysh> ", 7);
        if(fgets(line, SIZE, stdin) == NULL){
            exit(0);
        }
        char **parsedArgs = getcmd(line);
        char cmd[12] = "";
        char path[20] = "";
        strcpy(path, parsedArgs[0]);

        if(strlen(path) < 6){
            if(launch(path, parsedArgs) == 0){
                free(parsedArgs);
                path[0] = 0;
                cmd[0] = 0;
                continue;
            }
        }

        for(int i=0; i<strlen(path)-4; i++){
            cmd[i] = path[i+5];
        }
        strcpy(parsedArgs[0], cmd);

        launch(path, parsedArgs);
        free(parsedArgs);
        path[0] = 0;
        cmd[0] = 0;
    }
}

/**
 * @brief Batch mode: If a file has been provided in the invocation of the 
 * shell. Open the file and use the pointer to parse through the contents and 
 * provide lines to the getcmd() function. 
 * @param filename Takes in a filename provided by the user when starting 
 * the shell program.
 * @return int 
 */
int batch(char *filename){
    FILE *fptr;
    char line[SIZE];
    fptr = fopen(filename, "r");

    if (fptr == NULL) {
        char newline[3] = ".\n";
        strcat(filename, newline);
        write(2, "Error: Cannot open file ", 24);
        write(2, filename, strlen(filename));
		exit(1);
    }

    else {
        while(fgets(line, sizeof(line), fptr)!= NULL) {
            write(1, line, strlen(line));
            char **parsedArgs = getcmd(line);
            char cmd[12];
            strcpy(cmd, parsedArgs[0]);
            char path[20] = "";
            strcat(path, cmd);

            launch(path, parsedArgs);
            free(parsedArgs);
        }
    }

    fclose(fptr);
    return 0;
}

/**
 * @brief Main function of the shell program. Accepts in the given command line
 * arguments and while direct the program else where depending on if a batch
 * file has been provided by the user. If no file is provided the shell enters
 * interactive mode.
 * 
 * @param argc Number of commands passed to the shell program.
 * @param argv String of arguments passed that must be parsed.
 * @return int 
 */
int main(int argc, char *argv[]){

    if (argc == 1) {
        interactive();
    }

    else if (argc == 2) {
        char *array = argv[1];
        batch(array);
    }

    else if (argc > 2) {
        write(2, "Usage: mysh [batch-file]\n", 25);
        exit(1);
    }
}