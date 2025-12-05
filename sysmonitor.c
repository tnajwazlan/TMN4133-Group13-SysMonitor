/*
 * SysMonitor++ - Linux System Resource Monitoring Tool
 * TMN4133 System Programming Group Project
 * Member 1: Core Infrastructure & CPU Module
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <fcntl.h>

// Global variables
#define LOG_FILE "syslog.txt"
volatile sig_atomic_t keep_running = 1;

// Function prototypes
void displayMenu();
void getCPUUsage();
void getMemoryUsage();
void listTopProcesses();
void continuousMonitor(int interval);
void handleSignal(int sig);
void writeLog(const char* message);
void getCurrentTimestamp(char* buffer, size_t size);

/*
 * Main function - Entry point of the program
 */
int main(int argc, char *argv[]) {
    int choice;
    
    // Register signal handler for SIGINT (Ctrl+C)
    signal(SIGINT, handleSignal);
    
    // Log program start
    writeLog("=== SysMonitor++ Started ===");
    
    // Main menu loop
    while(keep_running) {
        displayMenu();
        printf("Enter your choice: ");
        
        if(scanf("%d", &choice) != 1) {
            // Clear invalid input
            while(getchar() != '\n');
            printf("Invalid input! Please enter a number.\n");
            continue;
        }
        
        printf("\n");
        
        switch(choice) {
            case 1:
                getCPUUsage();
                break;
            case 2:
                getMemoryUsage();
                break;
            case 3:
                listTopProcesses();
                break;
            case 4:
                printf("Enter refresh interval in seconds: ");
                int interval;
                if(scanf("%d", &interval) == 1 && interval > 0) {
                    continuousMonitor(interval);
                } else {
                    printf("Invalid interval!\n");
                }
                break;
            case 5:
                printf("Exiting SysMonitor++...\n");
                writeLog("=== SysMonitor++ Ended (User Exit) ===");
                keep_running = 0;
                break;
            default:
                printf("Invalid choice! Please select 1-5.\n");
        }
        
        if(choice >= 1 && choice <= 4) {
            printf("\nPress Enter to continue...");
            while(getchar() != '\n'); // Clear buffer
            getchar(); // Wait for Enter
        }
    }
    
    return 0;
}

/*
 * Display main menu
 */
void displayMenu() {
    printf("\n");
    printf("========================================\n");
    printf("       SysMonitor++ v1.0\n");
    printf("   System Resource Monitor\n");
    printf("========================================\n");
    printf("1. CPU Usage\n");
    printf("2. Memory Usage\n");
    printf("3. Top 5 Processes\n");
    printf("4. Continuous Monitoring\n");
    printf("5. Exit\n");
    printf("========================================\n");
}

/*
 * Get current timestamp for logging
 */
void getCurrentTimestamp(char* buffer, size_t size) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    strftime(buffer, size, "%Y-%m-%d %H:%M:%S", t);
}

/*
 * Write log entry to syslog.txt
 */
void writeLog(const char* message) {
    FILE *logfile = fopen(LOG_FILE, "a");
    if(logfile == NULL) {
        perror("Error opening log file");
        return;
    }
    
    char timestamp[64];
    getCurrentTimestamp(timestamp, sizeof(timestamp));
    
    fprintf(logfile, "[%s] %s\n", timestamp, message);
    fclose(logfile);
}

/*
 * Get CPU Usage from /proc/stat
 * Reads CPU statistics and calculates usage percentage
 */
void getCPUUsage() {
    int fd;
    char buffer[1024];
    ssize_t bytes_read;
    
    printf("========================================\n");
    printf("         CPU USAGE INFORMATION\n");
    printf("========================================\n");
    
    // Open /proc/stat using system call
    fd = open("/proc/stat", O_RDONLY);
    if(fd == -1) {
        perror("Error opening /proc/stat");
        writeLog("ERROR: Failed to read CPU usage");
        return;
    }
    
    // Read file content
    bytes_read = read(fd, buffer, sizeof(buffer) - 1);
    if(bytes_read == -1) {
        perror("Error reading /proc/stat");
        close(fd);
        return;
    }
    
    buffer[bytes_read] = '\0';
    close(fd);
    
    // Parse CPU line: cpu user nice system idle iowait irq softirq
    unsigned long long user, nice, system, idle, iowait, irq, softirq;
    char cpu_label[10];
    
    sscanf(buffer, "%s %llu %llu %llu %llu %llu %llu %llu",
           cpu_label, &user, &nice, &system, &idle, &iowait, &irq, &softirq);
    
    // Calculate CPU usage
    unsigned long long total_idle = idle + iowait;
    unsigned long long total_active = user + nice + system + irq + softirq;
    unsigned long long total = total_idle + total_active;
    
    // Simple calculation (for real-time, you'd need two samples)
    double usage_percent = 0.0;
    if(total > 0) {
        usage_percent = (double)total_active / total * 100.0;
    }
    
    // Display results
    printf("CPU Label    : %s\n", cpu_label);
    printf("User Time    : %llu\n", user);
    printf("System Time  : %llu\n", system);
    printf("Idle Time    : %llu\n", idle);
    printf("Total Active : %llu\n", total_active);
    printf("Total Time   : %llu\n", total);
    printf("\nCPU Usage    : %.2f%%\n", usage_percent);
    printf("========================================\n");
    
    // Log the result
    char log_msg[256];
    snprintf(log_msg, sizeof(log_msg), "CPU Usage: %.2f%%", usage_percent);
    writeLog(log_msg);
}

/*
 * Get Memory Usage from /proc/meminfo
 * TO BE IMPLEMENTED BY MEMBER 2
 */
void getMemoryUsage() {
    printf("========================================\n");
    printf("       MEMORY USAGE INFORMATION\n");
    printf("========================================\n");
    printf("This feature will be implemented by Member 2.\n");
    printf("========================================\n");
    
    writeLog("Memory Usage: Feature pending (Member 2)");
}

/*
 * List Top 5 Active Processes
 * TO BE IMPLEMENTED BY MEMBER 2
 */
void listTopProcesses() {
    printf("========================================\n");
    printf("         TOP 5 ACTIVE PROCESSES\n");
    printf("========================================\n");
    printf("This feature will be implemented by Member 2.\n");
    printf("========================================\n");
    
    writeLog("Top Processes: Feature pending (Member 2)");
}

/*
 * Continuous Monitoring Mode
 * TO BE IMPLEMENTED BY MEMBER 3
 */
void continuousMonitor(int interval) {
    printf("========================================\n");
    printf("      CONTINUOUS MONITORING MODE\n");
    printf("========================================\n");
    printf("This feature will be implemented by Member 3.\n");
    printf("Refresh interval: %d seconds\n", interval);
    printf("Press Ctrl+C to stop.\n");
    printf("========================================\n");
    
    writeLog("Continuous Monitor: Feature pending (Member 3)");
}

/*
 * Signal Handler for SIGINT (Ctrl+C)
 * Ensures graceful exit and log saving
 */
void handleSignal(int sig) {
    if(sig == SIGINT) {
        printf("\n\n========================================\n");
        printf("  Signal received: Ctrl+C\n");
        printf("  Exiting gracefully...\n");
        printf("  Saving logs...\n");
        printf("========================================\n");
        
        writeLog("=== SysMonitor++ Ended (SIGINT) ===");
        exit(0);
    }
}
