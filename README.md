# memdisk
Memdisk is an under development, experimental memory filesystem written in C. It works in a server-client fashion, 
by using System-V shared memory in order to allow inter-process communication.


## Installation

In order to install memdisk on your local machine, please execute the following commands:
```
./set-installation-path.sh <memdir-installation-path> [--create-dir]
make all
```

Notes: "--create-dir" flag creates the directory if it does not exist.
If your installation path is /usr/bin, then you will need to run `sudo make all`.

## Usage

### Daemon: 
To run the daemon in the foreground, please execute the following command:
```
memd <size> <KB|MB|GB>
```
Otherwise, if you would like to run in in the background:
```
nohup memd <size> <KB|MB|GB> &>/dev/null &
```
  
### Client:
Usage:
```
mem <command> [args...]
```

If you want to to execute the applications directly, add the `bin` folder to your PATH.

## Available commands

**mem ls** (list files)

**mem to <memdisk-filename> <new-filename>** (copy from RAM to disk)

**mem from <disk-filename> <new-filename>** (copy from disk to RAM)

**mem rm <filename>** (delete a file)

**mem quota** (show the available quota)

**mem touch <filename>** (create a new empty file)

**mem mkdir <dirname>** (create a new empty directory)

**mem pwd** (show current working directory)
