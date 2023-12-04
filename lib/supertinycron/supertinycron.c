
#define _ISOC99_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <signal.h>
#include <ctype.h>

#include "ccronexpr.h"

#ifndef VERSION
    #define VERSION "dev-build"
#endif

typedef struct { char *shell, *cmd, *schedule; int verbose; } TinyCronJob;

void output(const char *msg) {
    printf("[supertinycron] %s\n", msg);
}

void sigchld_handler(int signo) {
    (void) signo;
    /*while (waitpid(-1, NULL, WNOHANG) > 0);*/
}

void sig_handler(int signo) {
    if (signo == SIGTERM || signo == SIGINT) {
        output("terminated");
        _exit(0);
    }
}

int cron_system(const char *shell, const char *command) {
    int stdout_pipe[2], stderr_pipe[2];
    pid_t pid;

    if (pipe(stdout_pipe) != 0 || pipe(stderr_pipe) != 0) {
        perror("pipe");
        return -1;
    }

    pid = fork();
    if (pid == 0) {
        close(stdout_pipe[0]);
        close(stderr_pipe[0]);

        if (dup2(stdout_pipe[1], STDOUT_FILENO) == -1) {
            perror("dup2 stdout");
            return -1;
        }

        if (dup2(stderr_pipe[1], STDERR_FILENO) == -1) {
            perror("dup2 stderr");
            return -1;
        }

        close(stdout_pipe[1]);
        close(stderr_pipe[1]);

        execl(shell, shell, "-c", command, NULL);
        perror("execl");
        exit(EXIT_FAILURE);
    } else if (pid < 0) {
        perror("fork");
        return -1;
    } else {
        char buffer[4096];
        int nbytes;

        close(stdout_pipe[1]);
        close(stderr_pipe[1]);

        while ((nbytes = read(stdout_pipe[0], buffer, sizeof(buffer))) > 0) {
            write(STDOUT_FILENO, buffer, nbytes);
        }

        while ((nbytes = read(stderr_pipe[0], buffer, sizeof(buffer))) > 0) {
            write(STDERR_FILENO, buffer, nbytes);
        }

        close(stdout_pipe[0]);
        close(stderr_pipe[0]);

        int status;
        pid_t wpid = waitpid(pid, &status, 0);
        if (wpid == -1) {
            perror("waitpid");
            return -1;
        }
        if (WIFEXITED(status)) return WEXITSTATUS(status);
        else if (WIFSIGNALED(status)) return -WTERMSIG(status);
        return -1;
    }
}

TinyCronJob optsFromEnv() {
    TinyCronJob opts = {0, 0, 0, 0};
    if (getenv("TINYCRON_VERBOSE") != NULL) opts.verbose = 1;
    opts.shell = getenv("SHELL");
    if (!opts.shell) opts.shell = (char *)"/bin/sh";
    return opts;
}

void usage() {
    printf("Usage: supertinycron [expression] [command]\n");
    exit(EXIT_FAILURE);
}

void message(const char *err, const char *msg) {
    if (strlen(msg) == 0) output(err);
    else {
        char errMsg[512];
        snprintf(errMsg, sizeof(errMsg), "%s %s", msg, err);
        output(errMsg);
    }
}

void messageInt(int err, const char *msg) {
    if (err) message(strerror(err), msg);
}

void exitOnErr(int err, const char *msg) {
    if (err) {
        messageInt(err, msg);
        exit(EXIT_FAILURE);
    }
}

void run(TinyCronJob *job) {
    if (job->verbose) message(job->cmd, "running job:");

    messageInt(cron_system(job->shell, job->cmd), "job failed:");
}

int nap(TinyCronJob *job) {
    time_t current_time = time(NULL), next_run;

    cron_expr expr;
    const char* err = NULL;
    cron_parse_expr(job->schedule, &expr, &err);

    if (err) {
        message(err, "error parsing cron expression:");
        return 1;
    }

    next_run = cron_next(&expr, current_time);

    if (job->verbose) {
        char msg[512];
        struct tm *time_info = localtime(&next_run);
        strftime(msg, sizeof(msg), "%Y-%m-%d %H:%M:%S", time_info);
        message(msg, "next job scheduled for");
    }

    int sleep_duration = next_run - current_time;
    sleep(sleep_duration);
    return 0;
}

char* find_nth(const char* str, char ch, int n) {
    int count = 0;
    while (*str) {
        if (*str == ch && ++count == n) return (char*)str;
        str++;
    }
    return NULL;
}

void parse_line(char *line, TinyCronJob *job, int count) {
    job->schedule = line;
    job->cmd = find_nth(line, ' ', line[0] == '@' ? 1 : count);

    if (!job->cmd) {
        messageInt(1, "incomplete cron expression");
        exit(EXIT_FAILURE);
    }
    *job->cmd = '\0';
    ++job->cmd;
}
/*
int main(int argc, char *argv[]) {
    //signal(SIGCHLD, sigchld_handler);
    signal(SIGTERM, sig_handler);
    signal(SIGINT,  sig_handler);

    if (argc < 2 || strcmp(argv[1], "help") == 0) usage();

    if (strcmp(argv[1], "version") == 0) {
        printf("supertinycron version %s\n", VERSION);
        return EXIT_SUCCESS;
    }

    TinyCronJob job = optsFromEnv();

    int i, line_len = 0;
    for (i = 1; i < argc; i++) {
        line_len += strlen(argv[i]);
    }

    line_len += argc - 3;
    line_len += 1;

    char *line = (char *)malloc(line_len);
    if (!line) {
        perror("malloc");
        return EXIT_FAILURE;
    }

    strcpy(line, argv[1]);

    for (i = 2; i < argc; i++) {
        strcat(line, " ");
        strcat(line, argv[i]);
    }

    if (job.verbose) message(line, "line");

    parse_line(line, &job, 7);

    while (1) {
        if (nap(&job)) {
            perror("error creating job");
            break;
        }
        run(&job);
    }

    free(line);

    return EXIT_SUCCESS;
}
*/