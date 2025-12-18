#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <dirent.h>
#include <ctype.h>
#include <sys/stat.h>

#define MAX_PROC 1024

FILE *logFile;

/* ---------------- Utility Functions ---------------- */
void pressAnyKeyToContinue() {
    printf("\nPress Any Key to continue...");
    fflush(stdout);
    while (getchar() != '\n');
}

void flushInput() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {
        ;
    }
}

/* ---------------- Logging ---------------- */
void writeLog(const char *message) {
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);

    logFile = fopen("syslog.txt", "a");
    if (!logFile) {
        perror("fopen log");
        return;
    }

    fprintf(logFile, "[%02d-%02d-%d %02d:%02d:%02d] %s\n",
            tm->tm_mday, tm->tm_mon + 1, tm->tm_year + 1900,
            tm->tm_hour, tm->tm_min, tm->tm_sec, message);
    fclose(logFile);
}

/* ---------------- Signal Handling ---------------- */
void handleSignal(int sig) {
    (void)sig;
    writeLog("\nSession ended by user (Ctrl+C)");
    printf("\nExiting... Saving log.\n");
    exit(0);
}

/* ---------------- CPU Usage ---------------- */
void getCPUUsage() {
    char buffer[1024];
    unsigned long u1, n1, s1, i1;
    unsigned long u2, n2, s2, i2;

    int fd = open("/proc/stat", O_RDONLY);
    if (fd < 0) {
        perror("open /proc/stat");
        return;
    }
    read(fd, buffer, sizeof(buffer));
    close(fd);
    sscanf(buffer, "cpu %lu %lu %lu %lu", &u1, &n1, &s1, &i1);

    sleep(1);

    fd = open("/proc/stat", O_RDONLY);
    if (fd < 0) {
        perror("open /proc/stat");
        return;
    }
    read(fd, buffer, sizeof(buffer));
    close(fd);
    sscanf(buffer, "cpu %lu %lu %lu %lu", &u2, &n2, &s2, &i2);

    unsigned long total1 = u1 + n1 + s1 + i1;
    unsigned long total2 = u2 + n2 + s2 + i2;

    float usage = 100.0f * ((total2 - total1) - (i2 - i1)) / (total2 - total1);

    printf("CPU Usage: %.2f%%\n", usage);
    char log[64];
    sprintf(log, "CPU Usage: %.2f%%", usage);
    writeLog(log);
}

/* ---------------- Memory Usage ---------------- */
void getMemoryUsage() {
}

/* ---------------- Process List ---------------- */
typedef struct {
    int pid;
    char name[256];
    long cpu_time;
} Process;

void listTopProcesses() {
    DIR *dir = opendir("/proc");
    if (!dir) {
        perror("opendir /proc");
        return;
    }

    struct dirent *entry;
    Process proc[MAX_PROC];
    int count = 0;

    while ((entry = readdir(dir)) && count < MAX_PROC) {
        if (isdigit(entry->d_name[0])) {
            char statPath[300], commPath[300];
            sprintf(statPath, "/proc/%s/stat", entry->d_name);
            sprintf(commPath, "/proc/%s/comm", entry->d_name);

            FILE *sf = fopen(statPath, "r");
            FILE *cf = fopen(commPath, "r");
            if (!sf || !cf) {
                if (sf) fclose(sf);
                if (cf) fclose(cf);
                continue;
            }

            long utime, stime;
            fscanf(sf,
                   "%d %*s %*c %*d %*d %*d %*d %*d %*u %*u %*u %*u %*u %ld %ld",
                   &proc[count].pid, &utime, &stime);
            fgets(proc[count].name, sizeof(proc[count].name), cf);
            proc[count].name[strcspn(proc[count].name, "\n")] = 0;

            proc[count].cpu_time = utime + stime;
            count++;

            fclose(sf);
            fclose(cf);
        }
    }
    closedir(dir);

    for (int i = 0; i < count - 1; i++)
        for (int j = i + 1; j < count; j++)
            if (proc[j].cpu_time > proc[i].cpu_time) {
                Process t = proc[i];
                proc[i] = proc[j];
                proc[j] = t;
            }

    printf("Top 5 Processes (CPU Time)\nPID\tCPU\tName\n");
    char log[256] = "Top5 Processes: ";

    for (int i = 0; i < 5 && i < count; i++) {
        printf("%d\t%ld\t%s\n",
               proc[i].pid, proc[i].cpu_time, proc[i].name);
        char tmp[300];
        sprintf(tmp, "%d(%s) ", proc[i].pid, proc[i].name);
        strcat(log, tmp);
    }
    writeLog(log);
}

/* ---------------- Continuous Monitoring ---------------- */
void continuousMonitor(int interval) {
}

/* ---------------- Main ---------------- */
int main(int argc, char *argv[]) {
    signal(SIGINT, handleSignal);

    if (argc > 1) {
        if (strcmp(argv[1], "-m") == 0) {
            if (argc < 3) {
                printf("Error: missing parameter. Use -m [cpu/mem/proc]\n");
                return 1;
            }
            if (strcmp(argv[2], "cpu") == 0) getCPUUsage();
            else if (strcmp(argv[2], "mem") == 0) getMemoryUsage();
            else if (strcmp(argv[2], "proc") == 0) listTopProcesses();
            else printf("Invalid parameter. Use cpu/mem/proc\n");
            return 0;
        }
        else if (strcmp(argv[1], "-c") == 0) {
            if (argc < 3) {
                printf("Error: missing interval. Use -c [seconds]\n");
                return 1;
            }
            continuousMonitor(atoi(argv[2]));
            return 0;
        }
        else {
            printf("Invalid option. Use -m or -c\n");
            return 1;
        }
    }

    int choice;
    do {
        system("clear");
        printf("\n=== System Monitor Menu ===\n");
        printf("1. CPU Usage\n");
        printf("2. Memory Usage\n");
        printf("3. Top 5 Processes\n");
        printf("4. Continuous Monitoring (Press CTL + C to stop Continuous Monitoring)\n");
        printf("5. Exit\n");
        printf("Enter choice: ");
        if (scanf("%d", &choice) != 1) {
            flushInput();
            choice = -1;
        } else {
            flushInput();
        }
        
        switch (choice) {
            case 1:
                system("clear");
                getCPUUsage();
                pressAnyKeyToContinue();
                break;
            case 2:
                system("clear");
                getMemoryUsage();
                pressAnyKeyToContinue();
                break;
            case 3:
                system("clear");
                listTopProcesses();
                pressAnyKeyToContinue();
                break;
            case 4:
                continuousMonitor(2);
                break;
            case 5:
                writeLog("Exited Program");
                printf("\nBye!\n");
                pressAnyKeyToContinue();
                system("clear");
                break;
            default:
                printf("Invalid choice\n");
                pressAnyKeyToContinue();
                system("clear");
                break;
        }
    } while (choice != 5);

    return 0;
}