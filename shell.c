


#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

unsigned int count_args(char * command)
{
    unsigned int num_args = 0;

    // Create a copy because strtok destroys command
    char * command_copy = strdup(command);

    char* token = strtok(command_copy, " ");
  
    while (token != NULL) { 
        num_args++;
        token = strtok(NULL, " ");        
    }
    // If there is a trailing space, reduce the arg count by one 
    printf("count_args(...): command[strlen(command)-1] == %c\n", command[strlen(command)-1]);    
    if (command[strlen(command)-2] == ' ') // strlen - 2 because \n 
    {
        num_args--;
    }

    free(command_copy);

    return num_args;
}

char ** parse_command_args(char * command, ssize_t command_length, unsigned int * num_args_ptr)
{
    *num_args_ptr = count_args(command);

    printf("parse_command_args(...): num_args == %d\n", *num_args_ptr);
    char ** args = (char**) malloc(*num_args_ptr*sizeof(char*));
    unsigned int i = 0;

    // Create a copy because strtok destroys command
    char * command_copy = strdup(command);

    char* token = strtok(command_copy, " ");

    // && i < num_args because strtok will pick up a trailing space
    while (token != NULL && i < *num_args_ptr) {  
        printf("%s\n", token); 
        args[i] = token;
        token = strtok(NULL, " ");
        i++; 
    }
    return args;
}

unsigned int isChild(pid_t pid)
{
    if (pid == 0) { return 1; }
    else { return 0; }
}

// Execute the command with execvp
void execute_command(char ** args)
{
    execvp(args[0], args);
}
        
#define MAX_WORKING_DIRECTORY_SIZE 1024

int main()
{
    char * command = NULL;
    size_t command_allocated_size = 0;
    bool running = true;
    
    char * username = getlogin();
    char home_directory[MAX_WORKING_DIRECTORY_SIZE];
    sprintf(home_directory, "/home/%s", username);
    while (running)
    {
        char working_directory[MAX_WORKING_DIRECTORY_SIZE];
        getcwd(working_directory, MAX_WORKING_DIRECTORY_SIZE);
        
        char * home_directory_ptr = strstr(working_directory, home_directory);
        // TODO Turn this home_directory stuff into a function        
        if (home_directory_ptr != NULL)
        {
            strcpy(working_directory+1,working_directory+strlen(home_directory));
            working_directory[0] = '~';
        }    
        // get_current_dir_name TODO use this instead
        printf("%s@______:%s$ ", username, working_directory);        
        ssize_t num_characters_read = getline(&command, &command_allocated_size, stdin);
        
        unsigned int num_args = -1;
        
        char ** args = parse_command_args(command, num_characters_read, &num_args);
        //remove_whitespace_from_args(args);        
        for (int i = 0; i < num_args; i++)
        {
            printf("args[i] = %s\n",args[i]);
        }        

        pid_t child = fork();
        if (isChild(child))
        {
            execute_command(args);
        }
        else
        {

        }

    }
}