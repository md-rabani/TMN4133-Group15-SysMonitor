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