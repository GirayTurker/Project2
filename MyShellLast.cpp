#include <stdio.h>

#include <stdlib.h>

#include <unistd.h>

#include <sys/types.h>

#include <sys/wait.h>

#include <string.h>

#include <signal.h>

#include "myshell.h"

/*==================================== Declared Constants ====================================*/

#define ARGUMENTS 64

#define BUFFER_SIZE 1024

#define FNAME 256

#define CHRD " \t\n"

/*==================================== Declarded Variables ====================================*/

int numberOfArgs;

extern char **environ;

char inFLine[BUFFER_SIZE];

char spath[BUFFER_SIZE];

char path[BUFFER_SIZE];

char pathShell[BUFFER_SIZE];

char *args[ARGUMENTS];

char **arg;

char rdFile[3][FNAME];

char flagEle[3];

char *charRd[3] = {">>", ">", "<"};

/*==================================== Main function ====================================*/

int main(int argc, char **argv)
{

    signal(SIGINT, SIG_IGN);

    getcwd(spath, BUFFER_SIZE - 1);

    strcpy(pathShell, spath);

    strcat(pathShell, "/myshell");

    setenv("SHELL", pathShell, 1);

    if (argc > 2)
    {

        printError("Please Enter Only 1 (one) argument: Batc File Name");
    }

    else if (argc == 2)

    {

        batchFileReader(argv[1]);
    }

    while (!feof(stdin))
    {

        updatePath();

        printf("MyShell:");

        if (fgets(inFLine, BUFFER_SIZE, stdin))
        {

            turnItToken();

            int backRun = 0;

            if (numberOfArgs > 1 && !strcmp(args[numberOfArgs - 1], "&"))
            {

                args[numberOfArgs - 1] = (char *)0;

                backRun = 1;
            }

            if (backRun)
            {

                switch (fork())
                {

                case -1:

                    printError("fork error");

                    break;

                case 0:

                    setenv("PARENT", pathShell, 1);

                    runCommand();

                    exit(0);

                    break;
                }
            }

            else
            {

                runCommand();
            }
        }
    }

    return 0;
}

/*==================================== Print printError function ====================================*/

void printError(char msg[BUFFER_SIZE])
{

    printf("%s.\n", msg);

    exit(1);
}

/*==================================== Update Path Function ====================================*/

void updatePath()
{

    getcwd(path, BUFFER_SIZE - 1);
}

/*==================================== Tokenize Function ====================================*/

void turnItToken()
{

    char *tmp, *pt;

    int i;

    for (i = 0; i < 3; i++)
    {

        flagEle[i] = 0;

        if ((pt = strstr(inFLine, charRd[i])) != NULL)
        {

            tmp = strtok(pt + strlen(charRd[i]), CHRD);

            strcpy(rdFile[i], tmp);

            flagEle[i] = 1;

            *pt = '\0';

            if ((tmp = strtok(NULL, CHRD)) != NULL)

                strcat(inFLine, tmp);
        }
    }

    numberOfArgs = 1;

    arg = args;

    *arg++ = strtok(inFLine, CHRD);

    while ((*arg++ = strtok(NULL, CHRD)))

        numberOfArgs++;
}

/*==================================== signalChield Function ====================================*/
void signalChield(int param)
{

    signal(SIGINT, SIG_IGN);

    putchar('\n');
}

/*==================================== redirectionIO Function ====================================*/

void redirectionIO(int readAllowed)
{

    if (flagEle[0] == 1)

        freopen(rdFile[0], "a", stdout);

    if (flagEle[1] == 1)

        freopen(rdFile[1], "w", stdout);

    if (flagEle[2] == 1 && readAllowed == 1)

        freopen(rdFile[2], "r", stdin);
}

/*==================================== redirectionIOClose Function ====================================*/

void redirectionIOClose()
{

    if (flagEle[0] || flagEle[1])

        freopen("/dev/tty", "w", stdout);
}

/*==================================== runCommand Function ====================================*/

void runCommand()
{

    if (args[0])
    {

        if (!strcmp(args[0], "clr"))
        {

            system("clear");
        }

        else if (!strcmp(args[0], "dir"))
        {

            redirectionIO(0);

            char tmp[BUFFER_SIZE];

            strcpy(tmp, "ls -la ");

            if (args[1])

                strcat(tmp, args[1]);

            system(tmp);

            redirectionIOClose();
        }

        else if (!strcmp(args[0], "environ"))
        {

            redirectionIO(0);

            char **env = environ;

            while (*env)
            {

                printf("%s\n", *env);

                env++;
            }

            redirectionIOClose();
        }

        else if (!strcmp(args[0], "cd"))
        {

            if (!args[1])
            {

                printf("%s\n", path);
            }

            else
            {

                if (!chdir(args[1]))
                {

                    updatePath();

                    setenv("PWD", path, 1);
                }

                else
                {

                    printf("%s: No such file or directory exist\n", args[1]);
                }
            }
        }

        else if (!strcmp(args[0], "echo"))
        {

            redirectionIO(0);

            char *comment = (char *)malloc(BUFFER_SIZE);

            strcpy(comment, "");

            arg = &args[1];

            while (*arg)
            {

                strcat(comment, *arg++);

                strcat(comment, " ");
            }

            printf("%s\n", comment);

            memset(comment, 0, BUFFER_SIZE);

            free(comment);

            redirectionIOClose();
        }

        else if (!strcmp(args[0], "help"))
        {

            redirectionIO(1);

            char tmp[BUFFER_SIZE];

            strcpy(tmp, "more ");

            strcat(tmp, spath);

            strcat(tmp, "/readme");

            system(tmp);

            putchar('\n');

            redirectionIOClose();
        }

        else if (!strcmp(args[0], "pause"))
        {

            getpass("Press Enter to continue\n");
        }

        else if (!strcmp(args[0], "quit"))

            exit(0);

        else
        {

            int status;

            pid_t pid;

            signal(SIGINT, signalChield);

            switch (pid = fork())
            {

            case -1:

                printError("fork printError");

                break;

            case 0:

                setenv("PARENT", pathShell, 1);

                redirectionIO(1);

                if (execvp(args[0], args) == -1)

                    printError("command not found");

                exit(1);

                break;
            }

            fflush(stdout);

            waitpid(pid, &status, 0);
        }
    }
}

/*==================================== batchFileReader Function ====================================*/

void batchFileReader(char filename[FNAME])
{

    FILE *fp;

    int lineNo = 1;

    fp = fopen(filename, "r");

    if (fp == NULL)

        printError("Batch file does not exists");

    while (fgets(inFLine, BUFFER_SIZE, fp))
    {

        printf("%d. %s", lineNo++, inFLine);

        turnItToken();

        runCommand();

        putchar('\n');
    }

    fclose(fp);

    exit(0);
}