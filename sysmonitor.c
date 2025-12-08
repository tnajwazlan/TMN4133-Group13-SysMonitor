/*
 * SysMonitor++ - Linux System Resource Monitoring Tool
 * TMN4133 System Programming Group Project
 * Najwa: Main Code Structure & CPU Usage Module
 * Hanisah Zain: Memory Usage & Top Processes Implementation
 * Hanisah Ibrahim : README.md and Documentation
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <fcntl.h>
#include <dirent.h>
#include <ctype.h>

// Global variables
#define LOG_FILE "syslog.txt"
volatile sig_atomic_t keep_running = 1;

// Structure to hold process information
typedef struct {
    int pid;
    char name[256];
    unsigned long cpu_time;
    double cpu_percent;
} ProcessInfo;

// Function prototypes
void displayMenu();
void getCPUUsage();
void getMemoryUsage();
void listTopProcesses();
void continuousMonitor(int interval);
void handleSignal(int sig);
void writeLog(const char* message);
void getCurrentTimestamp(char* buffer, size_t size);
int compareProcesses(const void *a, const void *b);
int getProcessInfo(int pid, ProcessInfo *proc);

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
 * Reads memory statistics and calculates usage percentage
 */
void getMemoryUsage() {
    int fd;
    char buffer[4096];
    ssize_t bytes_read;
    
    printf("========================================\n");
    printf("       MEMORY USAGE INFORMATION\n");
    printf("========================================\n");
    
    // Open /proc/meminfo using system call
    fd = open("/proc/meminfo", O_RDONLY);
    if(fd == -1) {
        perror("Error opening /proc/meminfo");
        writeLog("ERROR: Failed to read memory usage");
        return;
    }
    
    // Read file content
    bytes_read = read(fd, buffer, sizeof(buffer) - 1);
    if(bytes_read == -1) {
        perror("Error reading /proc/meminfo");
        close(fd);
        return;
    }
    
    buffer[bytes_read] = '\0';
    close(fd);
    
    // Parse memory information
    unsigned long mem_total = 0, mem_free = 0, mem_available = 0;
    unsigned long buffers = 0, cached = 0, swap_total = 0, swap_free = 0;
    
    char *line = strtok(buffer, "\n");
    while(line != NULL) {
        if(strncmp(line, "MemTotal:", 9) == 0) {
            sscanf(line, "MemTotal: %lu", &mem_total);
        }
        else if(strncmp(line, "MemFree:", 8) == 0) {
            sscanf(line, "MemFree: %lu", &mem_free);
        }
        else if(strncmp(line, "MemAvailable:", 13) == 0) {
            sscanf(line, "MemAvailable: %lu", &mem_available);
        }
        else if(strncmp(line, "Buffers:", 8) == 0) {
            sscanf(line, "Buffers: %lu", &buffers);
        }
        else if(strncmp(line, "Cached:", 7) == 0) {
            sscanf(line, "Cached: %lu", &cached);
        }
        else if(strncmp(line, "SwapTotal:", 10) == 0) {
            sscanf(line, "SwapTotal: %lu", &swap_total);
        }
        else if(strncmp(line, "SwapFree:", 9) == 0) {
            sscanf(line, "SwapFree: %lu", &swap_free);
        }
        
        line = strtok(NULL, "\n");
    }
    
    // Calculate memory usage
    unsigned long mem_used = mem_total - mem_free - buffers - cached;
    unsigned long swap_used = swap_total - swap_free;
    
    double mem_usage_percent = (mem_total > 0) ? 
        ((double)mem_used / mem_total * 100.0) : 0.0;
    double swap_usage_percent = (swap_total > 0) ? 
        ((double)swap_used / swap_total * 100.0) : 0.0;
    
    // Display results (convert KB to MB)
    printf("PHYSICAL MEMORY:\n");
    printf("  Total Memory    : %.2f MB\n", mem_total / 1024.0);
    printf("  Used Memory     : %.2f MB\n", mem_used / 1024.0);
    printf("  Free Memory     : %.2f MB\n", mem_free / 1024.0);
    printf("  Available Memory: %.2f MB\n", mem_available / 1024.0);
    printf("  Buffers         : %.2f MB\n", buffers / 1024.0);
    printf("  Cached          : %.2f MB\n", cached / 1024.0);
    printf("  Memory Usage    : %.2f%%\n\n", mem_usage_percent);
    
    printf("SWAP MEMORY:\n");
    printf("  Total Swap      : %.2f MB\n", swap_total / 1024.0);
    printf("  Used Swap       : %.2f MB\n", swap_used / 1024.0);
    printf("  Free Swap       : %.2f MB\n", swap_free / 1024.0);
    printf("  Swap Usage      : %.2f%%\n", swap_usage_percent);
    printf("========================================\n");
    
    // Log the result
    char log_msg[256];
    snprintf(log_msg, sizeof(log_msg), 
             "Memory Usage: %.2f%% (%.2f/%.2f MB), Swap: %.2f%%", 
             mem_usage_percent, mem_used / 1024.0, mem_total / 1024.0,
             swap_usage_percent);
    writeLog(log_msg);
}

/*
 * Compare function for sorting processes by CPU time
 */
int compareProcesses(const void *a, const void *b) {
    ProcessInfo *p1 = (ProcessInfo *)a;
    ProcessInfo *p2 = (ProcessInfo *)b;
    
    // Sort in descending order (highest CPU time first)
    if(p2->cpu_time > p1->cpu_time) return 1;
    if(p2->cpu_time < p1->cpu_time) return -1;
    return 0;
}

/*
 * Read process information from /proc/[pid]/stat
 */
int getProcessInfo(int pid, ProcessInfo *proc) {
    char path[256];
    int fd;
    char buffer[2048];
    ssize_t bytes_read;
    
    // Read /proc/[pid]/stat
    snprintf(path, sizeof(path), "/proc/%d/stat", pid);
    fd = open(path, O_RDONLY);
    if(fd == -1) {
        return -1; // Process may have terminated
    }
    
    bytes_read = read(fd, buffer, sizeof(buffer) - 1);
    close(fd);
    
    if(bytes_read <= 0) {
        return -1;
    }
    
    buffer[bytes_read] = '\0';
    
    // Parse stat file
    // Format: pid (name) state ppid ... utime stime ...
    char *start = strchr(buffer, '(');
    char *end = strrchr(buffer, ')');
    
    if(start == NULL || end == NULL) {
        return -1;
    }
    
    // Extract process name
    int name_len = end - start - 1;
    if(name_len > 255) name_len = 255;
    strncpy(proc->name, start + 1, name_len);
    proc->name[name_len] = '\0';
    proc->pid = pid;
    
    // Parse CPU times (utime and stime are at positions 13 and 14)
    unsigned long utime, stime;
    char *ptr = end + 2; // Skip ") "
    int field = 1;
    
    while(*ptr && field < 13) {
        if(*ptr == ' ') field++;
        ptr++;
    }
    
    if(sscanf(ptr, "%lu %lu", &utime, &stime) == 2) {
        proc->cpu_time = utime + stime;
    } else {
        proc->cpu_time = 0;
    }
    
    return 0;
}

/*
 * List Top 5 Active Processes
 * Scans /proc directory and displays processes with highest CPU time
 */
void listTopProcesses() {
    DIR *proc_dir;
    struct dirent *entry;
    ProcessInfo processes[1024];
    int count = 0;
    
    printf("========================================\n");
    printf("         TOP 5 ACTIVE PROCESSES\n");
    printf("========================================\n");
    
    // Open /proc directory
    proc_dir = opendir("/proc");
    if(proc_dir == NULL) {
        perror("Error opening /proc directory");
        writeLog("ERROR: Failed to read process list");
        return;
    }
    
    // Scan all directories in /proc
    while((entry = readdir(proc_dir)) != NULL && count < 1024) {
        // Check if directory name is a number (PID)
        if(entry->d_type == DT_DIR && isdigit(entry->d_name[0])) {
            int pid = atoi(entry->d_name);
            
            if(getProcessInfo(pid, &processes[count]) == 0) {
                count++;
            }
        }
    }
    
    closedir(proc_dir);
    
    if(count == 0) {
        printf("No processes found!\n");
        printf("========================================\n");
        return;
    }
    
    // Sort processes by CPU time
    qsort(processes, count, sizeof(ProcessInfo), compareProcesses);
    
    // Display top 5 processes
    printf("%-8s %-30s %s\n", "PID", "PROCESS NAME", "CPU TIME");
    printf("----------------------------------------\n");
    
    int display_count = (count < 5) ? count : 5;
    char log_msg[512] = "Top 5 Processes: ";
    
    for(int i = 0; i < display_count; i++) {
        printf("%-8d %-30s %lu\n", 
               processes[i].pid, 
               processes[i].name, 
               processes[i].cpu_time);
        
        // Build log message (increased buffer size to handle long process names)
        char temp[300];
        snprintf(temp, sizeof(temp), "%s(%d) ", 
                 processes[i].name, processes[i].pid);
        strncat(log_msg, temp, sizeof(log_msg) - strlen(log_msg) - 1);
    }
    
    printf("========================================\n");
    printf("Total processes scanned: %d\n", count);
    printf("========================================\n");
    
    // Log the result
    writeLog(log_msg);
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
