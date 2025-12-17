# TMN4133-Group13-SysMonitor
Monitoring Tool using SysMonitor 

## Team Members:
- Ummie Najwa Binti Nor Azlan (85961)
- Nur Hanisah Binti Zain (85103)
- Nur Hanisah Binti Ibrahim (85101)
- Amisha Rittner Anak Tedi (83203)

## Compilation:
To compile the program, use the following command:

```gcc sysmonitor.c -o sysmonitor```

## Method #1 - Interactive Menu Mode:
- Run the program without any arguments:

 ```./sysmonitor```

- You will see a menu with 5 options:
    1. CPU Usage
    2. Memory Usage
    3. Top 5 Processes
    4. Continuous Monitoring
    5. Exit

- Enter the number (1-5) to select an option.

## Method #2 Command-Line Mode:
- Run specific functions directly using command-line flags

#### 1. Display CPU Usage

```./sysmonitor -m cpu```

#### 2. Display Memory Usage

```./sysmonitor -m mem```

#### 3. Display Top 5 Processes

```./sysmonitor -m proc```

#### 4. Continuous Monitoring (refresh every N seconds)

```./sysmonitor -c 2```

- Replace `2` with your desired refresh interval in seconds.
- Press `Ctrl + C` to stop continuous monitoring.


## Display Help:

```./sysmonitor -h```

## Log File:

- All operations are automatically logged to `syslog.txt` with timestamps.
- To view the log file:

```cat syslog.txt```


## GitHub Repository:

- Repository: `TMN4133-Group13-SysMonitor`
- URL: https://github.com/your-username/TMN4133-Group13-SysMonitor
