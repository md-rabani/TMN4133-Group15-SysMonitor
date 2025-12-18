# TMN4133-Group15-SysMonitor



#  🛠️ Installation & Compilation

##  1. Clone the repository:
### git clone https://github.com/your-username/your-repo-name.git
### cd your-repo-name

##  2. Compile the source code:
### gcc sysmonitor.c -o sysmonitor



#  💻 Usage

##  1. Interactive Menu Mode
### Simply run the executable without arguments to enter the menu:
### ./sysmonitor

### Menu Options:
### 1. CPU Usage: Shows current CPU load.
### 2. Memory Usage: Shows RAM statistics.
### 3. Top 5 Processes: Lists processes with highest CPU time.
### 4. Continuous Monitoring: Refreshes stats every 2 seconds.
### 5. Exit: Closes the program.

##  2. Command Line Arguments (CLI)
### You can use flags to get specific data instantly without entering the menu.

### Single Check Mode (`-m`):
### ./sysmonitor -m cpu    | Get CPU usage
### ./sysmonitor -m mem    | Get Memory usage
### ./sysmonitor -m proc   | Get Top 5 Processes

### Continuous Mode (`-c`):
### Run the monitor continuously with a custom refresh interval (in seconds).
### ./sysmonitor -c 2      | Refresh every 2 seconds

### (Press `Ctrl+C` to stop continuous monitoring).
