#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <sys/time.h>

#define MAX_PIPES 100
#define MAX_LINES 100
#define MAX_WORDS 100
#define MAX_CHARS 256
#define MAX_WORD_SIZE 50


typedef struct command_history {
    struct timeval start;
    struct timeval end;
    char command[MAX_CHARS];
    pid_t pid;
} command_history;

command_history history[MAX_CHARS];
int history_number = 0;



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


int parse_input(char *input, char cmds_arr[MAX_PIPES + 1][MAX_WORDS][MAX_WORD_SIZE], int word_count[MAX_PIPES + 1]) {
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

        num_cmds++;
        command = strtok_r(NULL, "|", &temp1);
    }

    return num_cmds;
}


void append_history(char *cmd[], struct timeval start, struct timeval end, pid_t pid){ 
    char full_command[MAX_CHARS] = "";
    
    for (int k = 0; cmd[k] != NULL; k++) {
        strcat(full_command, cmd[k]);
        
        if (cmd[k+1] != NULL) strcat(full_command, " ");
    }

    strncpy(history[history_number].command, full_command, MAX_CHARS - 1);
    history[history_number].command[MAX_CHARS - 1] = '\0'; 
    history[history_number].start = start;
    history[history_number].end = end;
    history[history_number].pid = pid;
    history_number++;
}


void display_history() {
    printf("\nShell History\n");
    printf("%-8s %-s\n", "Number", "Command");
    for (int i = 0; i < history_number; ++i) {
        printf("%-8d %-s\n", i + 1, history[i].command);
    }
}


void display_detailed_history() {
    printf("\nDetailed Shell History\n");
    printf("%-8s %-30s %-17s %-17s %-8s %-12s\n",
           "Number", "Command", "Start Time", "End Time", "PID", "Duration");
    for (int i = 0; i < history_number; ++i) {
        long long start_ms = (long long)history[i].start.tv_sec * 1000 + history[i].start.tv_usec/1000;
        long long end_ms   = (long long)history[i].end.tv_sec   * 1000 + history[i].end.tv_usec/1000;

        printf("%-8d %-20s %-15lld %-15lld %-8d %-12lldms\n", (i + 1), history[i].command, start_ms, end_ms, history[i].pid, (end_ms - start_ms));
    }

    printf("\nExiting Shell...... \n");
}


void launch(char cmds_arr[MAX_PIPES + 1][MAX_WORDS][MAX_WORD_SIZE], int word_count[MAX_PIPES + 1], int num_cmds) {

    int fd[2*(num_cmds-1)];
    for (int i = 0; i < num_cmds - 1; i++) {
        if (pipe(fd + i*2) < 0) {
            perror("Pipe failed");
            exit(1);
        }
    }

    pid_t pids[MAX_LINES];
    struct timeval start[MAX_LINES], end[MAX_LINES];
    char *all_exec_args[MAX_LINES][MAX_WORDS + 1]; 

    for (int i = 0; i < num_cmds; i++) {
        gettimeofday(&start[i], NULL); 
        
        char *exec_args[MAX_WORDS + 1];
        for (int k = 0; k < word_count[i]; k++) {
            exec_args[k] = cmds_arr[i][k];
        }
        
        exec_args[word_count[i]] = NULL;

        for (int k = 0; k <= word_count[i]; k++) {
            all_exec_args[i][k] = exec_args[k];
        }

        pids[i] = fork();
        if (pids[i] < 0) {
            perror("Fork failed");
            exit(1);
        }

        if (pids[i] == 0) {
            if (i > 0) {
                dup2(fd[(i-1)*2], STDIN_FILENO);
            }
            if (i < num_cmds - 1) {
                dup2(fd[i*2 + 1], STDOUT_FILENO);
            }

            int j = 0;
            while (j < 2*(num_cmds-1)) {
                close(fd[j]);
                j += 1;
            }

            execvp(exec_args[0], exec_args);
            perror("execvp failed");
            exit(1);
        }
        
    }

    for (int i = 0; i < 2*(num_cmds-1); i++) {
        close(fd[i]);
    }

    for(int i = 0;i < num_cmds; ++i){ 
        waitpid(pids[i], 0, 0);
        gettimeofday(&end[i], NULL); 
        append_history(all_exec_args[i], start[i], end[i], pids[i]);
    }
}


static void signal_handler(int signum){ 
    if(signum == SIGINT){ 
        display_detailed_history(); 
        exit(0);
    }
}


int main() {
    struct sigaction sig; 
    memset(&sig,0, sizeof(sig)); 
    sig.sa_handler = signal_handler;
    sigaction(SIGINT, &sig, NULL); 
    
    char input[MAX_CHARS];
    char cmds_arr[MAX_PIPES+1][MAX_WORDS][MAX_WORD_SIZE];
    int word_count[MAX_PIPES+1];
    
    while (1) {
        printf(">> Simple Shell: ~$ ");
        if (fgets(input, sizeof(input), stdin) == NULL) break;
        
        input[strcspn(input, "\n")] = '\0';
        trim_string(input);
        
        if (input[0] == '\0') continue;

        int num_cmds = parse_input(input, cmds_arr, word_count);

        if (num_cmds == 0) continue;

        if (strcmp(cmds_arr[0][0], "history") == 0) { 
            display_history(); 
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
