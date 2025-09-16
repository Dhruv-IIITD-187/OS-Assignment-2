#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <sys/time.h>

#define MAX_CHARS 150
#define MAX_CMDS 30
#define MAX_WORDS 30
#define MAX_WORD_SIZE 50

// Structs and uninitialised variables for storing short and long format history
typedef struct detailed_command_history {
    struct timeval start;
    struct timeval end;
    char command[MAX_CHARS];
    pid_t pid;
} detailed_command_history;

detailed_command_history detailed_history[MAX_CHARS];
int history_number = 0;

char* short_history[MAX_CHARS];
int hist_num = 0;

// Custom handler for trimming a string of its leading and trailing whitespaces
void trim_string(char *str) {
    char *start = str;
    while (*start == ' ' || *start == '\t') {
        start++;
    }

    char *end = str + strlen(str) - 1;
    while (end >= str && (*end == ' ' || *end == '\t')) {
        end--;
    }

    ssize_t str_len = end - start + 1;
    memmove(str, start, str_len);
    str[str_len] = '\0';
}


// Parses the raw user CLI string to a refined 3d array consisting of commands separated by pipes --> words/tokens in each command --> each word as a char array
int parse_input(char *input, char cmds_arr[MAX_CMDS][MAX_WORDS][MAX_WORD_SIZE], int word_count[MAX_CMDS]) {
    int num_cmds = 0;

    char *temp1 = NULL;
    char *command = strtok_r(input, "|", &temp1);

    while (command) {
        trim_string(command);

        word_count[num_cmds] = 0;
        char *temp2 = NULL;
        char *token = strtok_r(command, " \t", &temp2);

        while (token) {
            trim_string(token);
            
            strncpy(cmds_arr[num_cmds][word_count[num_cmds]], token, MAX_WORD_SIZE - 1);
            cmds_arr[num_cmds][word_count[num_cmds]][MAX_WORD_SIZE - 1] = '\0';
            word_count[num_cmds] += 1;
            
            token = strtok_r(NULL, " \t", &temp2);
        }

        num_cmds += 1;
        command = strtok_r(NULL, "|", &temp1);
    }

    return num_cmds;
}


// Handler for short form command history
void append_short_history(char* commands[MAX_CMDS][MAX_WORDS+1], int num_cmds ) {
    char full_command[MAX_CHARS] = "";

    for (int i = 0; i < num_cmds; i++) {
        for (int j = 0; commands[i][j] != NULL; j++) {
            strcat(full_command, commands[i][j]);
            if (commands[i][j+1] != NULL) strcat(full_command, " ");
        }
        if (i < num_cmds - 1) strcat(full_command, " | ");
    }

    short_history[hist_num] = strdup(full_command);
    hist_num++;
}


// Handler for detailed command history
void append_detailed_history(char* cmd[MAX_WORDS+1], struct timeval start, struct timeval end, pid_t pid){ 
    char full_command[MAX_CHARS] = "";
    
    for (int k = 0; cmd[k] != NULL; k++) {
        strcat(full_command, cmd[k]);
        if (cmd[k+1] != NULL) strcat(full_command, " ");
    }

    strncpy(detailed_history[history_number].command, full_command, MAX_CHARS - 1);
    detailed_history[history_number].command[MAX_CHARS - 1] = '\0'; 
    detailed_history[history_number].start = start;
    detailed_history[history_number].end = end;
    detailed_history[history_number].pid = pid;
    history_number++;
}


// Displays user typed commands. Called when user types "history"
void display_short_history() {
    printf("\nShell History ==>\n");
    printf("%-8s %-s\n", "Number", "Command");
    for (int i = 0; i < hist_num; ++i) {
        printf("%-8d %-s\n", i + 1, short_history[i]);
    }
}


// Displays command history along with details like pid, time of execution etc. Called when program is exited
void display_detailed_history() {
    printf("\nDetailed Shell History ==>\n");
    printf("%-8s %-40s %-17s %-17s %-8s %-12s\n",
           "Number", "Command", "Start Time", "End Time", "PID", "Duration");
    for (int i = 0; i < history_number; ++i) {
        long long start_ms = (long long)detailed_history[i].start.tv_sec * 1000 + detailed_history[i].start.tv_usec/1000;
        long long end_ms   = (long long)detailed_history[i].end.tv_sec   * 1000 + detailed_history[i].end.tv_usec/1000;

        printf("%-8d %-40s %-17lld %-17lld %-8d %-12lldms\n", (i + 1), detailed_history[i].command, start_ms, end_ms, detailed_history[i].pid, (end_ms - start_ms));
    }

    printf("\nExiting Shell...... \n");

    for (int i = 0; i < hist_num; ++i) {
        free(short_history[i]);
    }
}


// Core execution logic of the shell
void launch(char cmds_arr[MAX_CMDS][MAX_WORDS][MAX_WORD_SIZE], int word_count[MAX_CMDS], int num_cmds) {

    // Initialising (number of commands - 1) pipe objects
    int fd[2*(num_cmds-1)];
    for (int i = 0; i < num_cmds - 1; i++) {
        if (pipe(fd + i*2) < 0) {
            perror("Pipe failed");
            exit(1);
        }
    }

    pid_t pids[MAX_CMDS];
    struct timeval start[MAX_CMDS], end[MAX_CMDS];
    char* all_exec_args[MAX_CMDS][MAX_WORDS+1]; 

    // Iterating over each command in the pipe
    for (int i = 0; i < num_cmds; i++) {
        gettimeofday(&start[i], NULL); 
        
        char* exec_args[MAX_WORDS+1];
        for (int k = 0; k < word_count[i]; k++) {
            exec_args[k] = cmds_arr[i][k];
        }
        
        exec_args[word_count[i]] = NULL; // Last char = NULL for execvp()

        for (int k = 0; k <= word_count[i]; k++) {
            all_exec_args[i][k] = exec_args[k];
        }

        pids[i] = fork(); // Creating Child Process
        if (pids[i] < 0) {
            perror("Fork failed");
            exit(1);
        }

        // Connecting pipes to std input output of the child process
        if (pids[i] == 0) {
            if (i > 0) {
                dup2(fd[(i-1)*2], STDIN_FILENO);
            }
            if (i < num_cmds - 1) {
                dup2(fd[i*2 + 1], STDOUT_FILENO);
            }

            int j = 0;
            while (j < 2*(num_cmds-1)) { // Closing fds since pipes are already connected above
                close(fd[j]);
                j += 1;
            }

            execvp(exec_args[0], exec_args); // Command execution using execvp
            perror("execvp failed");
            exit(1);
        }
        
    }

    for (int i = 0; i < 2*(num_cmds-1); i++) {
        close(fd[i]);
    }

    for(int i = 0;i < num_cmds; ++i){ // Parent waiting for child process, and handling history management
        waitpid(pids[i], 0, 0);
        gettimeofday(&end[i], NULL); 
        append_detailed_history(all_exec_args[i], start[i], end[i], pids[i]);
    }

    append_short_history(all_exec_args, num_cmds);
}


// Handler for Ctrl+C action
static void signal_handler(int signum){ 
    // On Ctrl+C (SIGINT), detailed hsitory will be displayed and the program will gracefuly exit
    if(signum == SIGINT){ 
        display_detailed_history(); 
        exit(0);
    }
}


int main() {
    // Signal handling for SIGINT signal (Ctrl + C)
    struct sigaction sig; 
    memset(&sig,0, sizeof(sig)); 
    sig.sa_handler = signal_handler;
    sigaction(SIGINT, &sig, NULL); 
    
    char input[MAX_CHARS];
    char cmds_arr[MAX_CMDS][MAX_WORDS][MAX_WORD_SIZE];
    int word_count[MAX_CMDS];
    
    while (1) {
        printf(">> Simple Shell: ~$ ");
        if (fgets(input, sizeof(input), stdin) == NULL) break;
        
        input[strcspn(input, "\n")] = '\0';
        trim_string(input);
        
        if (input[0] == '\0') continue;

        int num_cmds = parse_input(input, cmds_arr, word_count);

        if (num_cmds == 0) continue;

        if (strcmp(cmds_arr[0][0], "history") == 0) { 
            display_short_history(); 
            continue; 
        }

        if (strcmp(cmds_arr[0][0], "exit") == 0) { 
            display_detailed_history();
            break;
        } 

        launch(cmds_arr, word_count, num_cmds);
    }
    return 0;
}
