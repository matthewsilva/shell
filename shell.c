#include <pwd.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

void substitute_home_directory(char * working_directory, const char* home_directory)
{
    char * home_directory_ptr = strstr(working_directory, home_directory);
    if (home_directory_ptr != NULL)
    {
        strcpy(working_directory+1,working_directory+strlen(home_directory));
        working_directory[0] = '~';
    }    
}

bool is_whitespace(char c)
{
    return c == ' ' || c == '\t' || c == '\n';
}

unsigned int count_args(char * command)
{
    unsigned int num_args = 0;
    unsigned int i = 0;
    // Get to first argument
    while (is_whitespace(command[i]))
    {
        i++;
    }
    // While not at the end
    while(command[i] != '\0')
    {
        // Parse one argument
        while(!is_whitespace(command[i]) && command[i] != '\0')
        {
            i++;
        } 
        num_args++;
        // Get to the next argument
        while (is_whitespace(command[i]))
        {
            i++;
        }
    }
    return num_args;
}

char ** parse_command_args(char * command, ssize_t command_length, unsigned int * num_args_ptr)
{
    *num_args_ptr = count_args(command);

    // Add one because the last arg must be NULL
    char ** args = (char**) malloc(*num_args_ptr*sizeof(char*)+1);
    unsigned int i = 0;

    // Create a copy because strtok destroys command
    char * command_copy = strdup(command);

    char* token = strtok(command_copy, " ");

    // && i < num_args because strtok will pick up a trailing space
    while (token != NULL && i < *num_args_ptr) {  
        args[i] = token;
        token = strtok(NULL, " ");
        i++; 
    }

    // Set last arg to NULL
    args[i] = NULL;

    return args;
}

unsigned int is_child(pid_t pid)
{
    if (pid == 0) { return 1; }
    else { return 0; }
}

// Execute the command with execvp
void execute_command(char ** args)
{
    execvp(args[0], args);
}
        
#define MAX_WORKING_DIRECTORY_SIZE 256
#define MAX_HOSTNAME_SIZE 256

// ANSI Color Codes
#define BLACK "\x1b[30m"
#define RED "\x1b[31m"
#define GREEN "\x1b[32m"
#define LIGHT_GREEN "\x1b[92m"
#define YELLOW "\x1b[33m"
#define BLUE "\x1b[34m"
#define LIGHT_BLUE "\x1b[94m"
#define MAGENTA "\x1b[35m"
#define CYAN "\x1b[36m"
#define WHITE "\x1b[37m"
#define RESET "\x1b[0m"



int main()
{
    char * command = NULL;
    size_t command_allocated_size = 0;
    bool running = true;
    
    char * username = getlogin();

    char hostname[MAX_HOSTNAME_SIZE];
    gethostname(hostname, MAX_HOSTNAME_SIZE);

    uid_t user_uid = getuid();
    struct passwd *user_passwd = getpwuid(user_uid);
    const char *home_directory = user_passwd->pw_dir;

    char working_directory[MAX_WORKING_DIRECTORY_SIZE];

    while (running)
    {
        getcwd(working_directory, MAX_WORKING_DIRECTORY_SIZE);        
        substitute_home_directory(working_directory, home_directory);
        
        printf(LIGHT_GREEN "%s@%s" WHITE ":" LIGHT_BLUE "%s" WHITE "$ " RESET, username, hostname, working_directory);        
        ssize_t num_characters_read = getline(&command, &command_allocated_size, stdin);
        
        // Remove the trailing newline from the command
        command[strlen(command)-1] = '\0';

        unsigned int num_args = -1;
        
        char ** args = parse_command_args(command, num_characters_read, &num_args);
        

        pid_t child = fork();
        if (is_child(child))
        {
            execute_command(args);
        }
        else
        {
            int child_exit_status;
            waitpid(child, &child_exit_status, 0);
        }

    }
}