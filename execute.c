#include "oshell.h"
#include "execute.h"
#include "errors.h"
#include "parser.h"
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <limits.h>

static char prev_dir[PATH_MAX] = "";


// Simple linked list for aliases
typedef struct alias {
    char *name;
    char *value;
    struct alias *next;
} alias_t;

static alias_t *alias_list = NULL;

// Internal path list
static char *paths[64] = {NULL};

void __attribute__((constructor)) init_paths() {
    paths[0] = strdup("/bin");
    paths[1] = strdup("/usr/bin");
}

// Helper: find alias
static char* get_alias(const char *name) {
    alias_t *cur = alias_list;
    while (cur) {
        if (strcmp(cur->name, name) == 0)
            return cur->value;
        cur = cur->next;
    }
    return NULL;
}

// Helper: set alias
static void set_alias(const char *name, const char *value) {
    alias_t *cur = alias_list;
    while (cur) {
        if (strcmp(cur->name, name) == 0) {
            free(cur->value);
            cur->value = strdup(value);
            return;
        }
        cur = cur->next;
    }
    alias_t *new_alias = malloc(sizeof(alias_t));
    new_alias->name = strdup(name);
    new_alias->value = strdup(value);
    new_alias->next = alias_list;
    alias_list = new_alias;
}

// Helper: expand environment vars and $$, $?
static void expand_variables(command_t *cmd) {
    for (int i = 0; cmd->argv[i]; i++) {
        if (cmd->argv[i][0] == '$') {
            if (strcmp(cmd->argv[i], "$?") == 0) {
                char buf[16];
                snprintf(buf, sizeof(buf), "%d", last_status);
                free(cmd->argv[i]);
                cmd->argv[i] = strdup(buf);
            } else if (strcmp(cmd->argv[i], "$$") == 0) {
                char buf[16];
                snprintf(buf, sizeof(buf), "%d", getpid());
                free(cmd->argv[i]);
                cmd->argv[i] = strdup(buf);
            } else {
                char *val = getenv(cmd->argv[i]+1);
                free(cmd->argv[i]);
                cmd->argv[i] = val ? strdup(val) : strdup("");
            }
        }
    }
}

// Built-in handlers
static int handle_builtin(command_t *cmd) {
    if (!cmd->argv[0]) return 0;

    // exit
    if (strcmp(cmd->argv[0], "exit") == 0) {
        if (cmd->argv[1] && cmd->argv[2]) { print_error(); last_status=1; return 1; }
        int code = 0;
        if (cmd->argv[1]) code = atoi(cmd->argv[1]);
        exit(code);
    }

    // cd
    if (strcmp(cmd->argv[0], "cd") == 0) {
        if (cmd->argv[1] && cmd->argv[2]) { print_error(); last_status=1; return 1; }
        char *dir = cmd->argv[1];
        if (!dir) dir = getenv("HOME");
        
        char target_dir[PATH_MAX];
        
        if (dir && strcmp(dir, "-") == 0) {
            if (strlen(prev_dir) == 0) {
                print_error();
                last_status = 1;
                return 1;
            }
            strcpy(target_dir, prev_dir);
            printf("%s\n", target_dir);
        } else {
            if (!dir) dir = "."; // Should catch above, but safety
            strcpy(target_dir, dir);
        }

        char cwd[PATH_MAX];
        if (!getcwd(cwd, sizeof(cwd))) {
            print_error();
            last_status = 1;
            return 1;
        }

        if (chdir(target_dir) != 0) { 
            print_error(); 
            last_status = 1; 
            return 1; 
        }

        // Success: update previous directory
        strcpy(prev_dir, cwd);
        
        // Update PWD environment variable
        char new_cwd[PATH_MAX];
        if (getcwd(new_cwd, sizeof(new_cwd))) {
            setenv("PWD", new_cwd, 1);
        }
        
        last_status = 0;
        return 1;
    }

    // env
    if (strcmp(cmd->argv[0], "env") == 0) {
        extern char **environ;
        for (int i = 0; environ[i]; i++)
            printf("%s\n", environ[i]);
        last_status = 0;
        return 1;
    }

    // setenv
    if (strcmp(cmd->argv[0], "setenv") == 0) {
        if (!cmd->argv[1] || !cmd->argv[2] || cmd->argv[3]) { print_error(); last_status=1; return 1; }
        if (setenv(cmd->argv[1], cmd->argv[2], 1) != 0) { print_error(); last_status=1; }
        else last_status=0;
        return 1;
    }

    // unsetenv
    if (strcmp(cmd->argv[0], "unsetenv") == 0) {
        if (!cmd->argv[1] || cmd->argv[2]) { print_error(); last_status=1; return 1; }
        if (unsetenv(cmd->argv[1]) != 0) { print_error(); last_status=1; }
        else last_status=0;
        return 1;
    }

    // alias
    if (strcmp(cmd->argv[0], "alias") == 0) {
        if (!cmd->argv[1]) { // print all aliases
            alias_t *cur = alias_list;
            while (cur) {
                printf("%s='%s'\n", cur->name, cur->value);
                cur = cur->next;
            }
        } else { // set alias
            for (int i=1; cmd->argv[i]; i++) {
                char *eq = strchr(cmd->argv[i], '=');
                if (eq) {
                    *eq = '\0';
                    set_alias(cmd->argv[i], eq+1);
                } else {
                    char *val = get_alias(cmd->argv[i]);
                    if (val) printf("%s='%s'\n", cmd->argv[i], val);
                }
            }
        }
        last_status=0;
        return 1;
    }

    // path
    if (strcmp(cmd->argv[0], "path") == 0) {
        if (!cmd->argv[1]) {
            // Display paths
            char buffer[1024] = "";
            for (int i=0; paths[i]; i++) {
                if (i > 0) strcat(buffer, ":");
                strcat(buffer, paths[i]);
                printf("%s ", paths[i]); // Print space separated as per common shell specs or just verify logic
            }
            printf("\n");
        } else {
            // Modify paths
            for (int i=0; i<64; i++) {
                free(paths[i]);
                paths[i]=NULL;
            }
            for (int i=1; cmd->argv[i]; i++)
                paths[i-1] = strdup(cmd->argv[i]);
        }
        last_status = 0;
        return 1;
    }

    return 0; // not a built-in
}

// Search external command in internal paths
static char* find_command(const char *name) {
    if (strchr(name, '/')) return strdup(name); // absolute/relative
    for (int i=0; paths[i]; i++) {
        char buf[1024];
        snprintf(buf, sizeof(buf), "%s/%s", paths[i], name);
        if (access(buf, X_OK)==0) return strdup(buf);
    }
    return NULL;
}

int execute_commands(command_t *cmds, int count)
{
    int prev_pipe_read = -1;
    pid_t pids[MAX_COMMANDS];
    int pcount = 0;

    for (int i=0; i<count; i++) {
        if (!cmds[i].argv || !cmds[i].argv[0]) continue;

        expand_variables(&cmds[i]);

        // check alias
        char *alias_val = get_alias(cmds[i].argv[0]);
        if (alias_val) {
            // Expand alias: "ll" -> "ls -l" means replacing "ll" with "ls", "-l"
            // We need to tokenize the alias value
            char *val_copy = strdup(alias_val);
            char *tokens[MAX_ARGS];
            int t_count = 0;
            char *tok = strtok(val_copy, " ");
            while (tok && t_count < MAX_ARGS) {
                tokens[t_count++] = tok;
                tok = strtok(NULL, " ");
            }

            if (t_count > 0) {
                // Calculate current args count
                int current_argc = 0;
                while (cmds[i].argv[current_argc]) current_argc++;

                // New size: (t_count) + (current_argc - 1) + 1 for NULL
                // Check overflow
                if (t_count + current_argc - 1 < MAX_ARGS) {
                    char **new_argv = malloc(sizeof(char*) * (MAX_ARGS + 1));
                    
                    int new_idx = 0;
                    // Copy alias tokens
                    for (int k=0; k<t_count; k++) {
                        new_argv[new_idx++] = strdup(tokens[k]);
                    }
                    // Copy remaining args (skip argv[0] which was the alias name)
                    for (int k=1; k<current_argc; k++) {
                        new_argv[new_idx++] = cmds[i].argv[k]; 
                        cmds[i].argv[k] = NULL; // Take ownership, prevent double free if we free old argv array only
                    }
                    new_argv[new_idx] = NULL;

                    // Free old argv[0]
                    free(cmds[i].argv[0]);
                    // Free old argv array wrapper
                    free(cmds[i].argv);
                    
                    cmds[i].argv = new_argv;
                }
            }
            free(val_copy);
        }

        // built-in commands
        if (handle_builtin(&cmds[i])) continue;

        // conditional execution
        if (i>0) {
            if (cmds[i-1].op==OP_AND && last_status!=0) continue;
            if (cmds[i-1].op==OP_OR && last_status==0) continue;
        }

        int pipefd[2];
        if (cmds[i].pipe_out && pipe(pipefd)<0) { print_error(); last_status=1; continue; }

        pid_t pid = fork();
        if (pid<0) { print_error(); last_status=1; continue; }

        if (pid==0) { // child
            // Reset signal handling to default
            signal(SIGINT, SIG_DFL);

            if (prev_pipe_read!=-1) { dup2(prev_pipe_read, STDIN_FILENO); close(prev_pipe_read); }
            if (cmds[i].pipe_out) { dup2(pipefd[1], STDOUT_FILENO); close(pipefd[0]); close(pipefd[1]); }
            if (cmds[i].redirect && !cmds[i].pipe_out) {
                int fd = open(cmds[i].redirect, O_WRONLY|O_CREAT|O_TRUNC, 0644);
                if (fd<0) { print_error(); exit(1); }
                dup2(fd, STDOUT_FILENO);
                close(fd);
            }

            char *cmd_path = find_command(cmds[i].argv[0]);
            if (!cmd_path) { print_error(); exit(127); }
            execv(cmd_path, cmds[i].argv);
            print_error();
            exit(1);
        }

        pids[pcount++] = pid;

        if (prev_pipe_read!=-1) close(prev_pipe_read);
        prev_pipe_read = cmds[i].pipe_out ? pipefd[0] : -1;
        if (cmds[i].pipe_out) close(pipefd[1]);

        if (!cmds[i].pipe_out && !cmds[i].background) {
            for (int j=0;j<pcount;j++){
                int status;
                waitpid(pids[j], &status, 0);
                last_status = WIFEXITED(status) ? WEXITSTATUS(status) : 1;
            }
            pcount=0;
        }
    }

    if (prev_pipe_read!=-1) close(prev_pipe_read);
    return last_status;
}


