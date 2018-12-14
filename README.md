# memdisk
Memdisk is an under development memory filesystem written in C. It works in a server-client fashion, 
by using System-V shared memory in order to allow inter-process communication.

## Usage

### Daemon: 
  * Foreground:
  `memd <size> <KB|MB|GB>`
  * Background:
  `nohup memd <size> <KB|MB|GB> &>/dev/null &`
  
### Client:
  `memc <command-id> <arg1> [arg2]`

If you want to to execute the applications directly, add the `bin` folder to your PATH.

## Available commands

0 - **ls** (list files)

1 - *reserved*

2 - *reserved*

3 - **memto** (copy from RAM to disk)

4 - **memfrom** (copy from disk to RAM)

5 - **rm** (delete a file)

6 - **quota** (show the available quota)

7 - **touch** (create a new empty file)

8 - **mkdir** (create a new empty directory)


## Scripts

Of course, you can use the available scripts to directly issue a command without remembering its ID.
Scripts are written in bash and need +x flag in order to be executed.
