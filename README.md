# memdisk
Memdisk is an under development memory filesystem written in C. It works in a server-client fashion, 
by using System-V shared memory in order to allow inter-process communication.

## Usage

Daemon: 

  ./memd <size> <KB|MB|GB>
  
  or (in background):
  
  nohup ./memd <size> <KB|MB|GB> &>/dev/null &
  
Client:
  
  ./memc <action-id> <arg1> [arg2]
