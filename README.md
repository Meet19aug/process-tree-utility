# process-tree-utility

## Link to code understanding and Demo 
https://drive.google.com/file/d/1eEz_ENnDnIkHMWFZEpD7GFqQEbLRPAZo/view?usp=drive_link


## Overview
This repository contains a C program named `prc24s` which searches for processes in the process tree rooted at a specified process and performs various actions based on input parameters.

## Features
- **Process Information:** Lists the PID and PPID of a specified process.
- **Process Management:** Kills, stops, or continues processes and their descendants.
- **Descendant Information:** Lists immediate and non-direct descendants, siblings, zombie processes, orphan processes, etc.

## Command Line Arguments
- `prc24s [root_process] [process_id]`: Lists the PID and PPID of `process_id` if it belongs to the process tree rooted at `root_process`.
- `prc24s -dx [root_process] [process_id]`: The `root_process` kills all its descendants using `SIGKILL`.
- `prc24s -dt [root_process] [process_id]`: The `root_process` sends `SIGSTOP` to all its descendants.
- `prc24s -dc [root_process] [process_id]`: The `root_process` sends `SIGCONT` to all its paused descendants.
- `prc24s -rp [root_process] [process_id]`: The `root_process` kills `process_id`.
- `prc24s -nd [root_process] [process_id]`: Lists the PIDs of all the non-direct descendants of `process_id`.
- `prc24s -dd [root_process] [process_id]`: Lists the PIDs of all the immediate descendants of `process_id`.
- `prc24s -sb [root_process] [process_id]`: Lists the PIDs of all the sibling processes of `process_id`.
- `prc24s -bz [root_process] [process_id]`: Lists the PIDs of all the sibling processes of `process_id` that are defunct.
- `prc24s -zd [root_process] [process_id]`: Lists the PIDs of all descendants of `process_id` that are defunct.
- `prc24s -od [root_process] [process_id]`: Lists the PIDs of all descendants of `process_id` that are orphans.
- `prc24s -gc [root_process] [process_id]`: Lists the PIDs of all the grandchildren of `process_id`.
- `prc24s -sz [root_process] [process_id]`: Prints the status of `process_id` (Defunct/Not Defunct).
- `prc24s -so [root_process] [process_id]`: Prints the status of `process_id` (Orphan/Not Orphan).
- `prc24s -kz [root_process] [process_id]`: Kills the parents of all zombie processes that are descendants of `process_id`.

## Usage Examples
- List process information: `$ prc24s 1004 1009`
- Kill all descendants: `$ prc24s -dx 1004 1009`
- List non-direct descendants: `$ prc24s -nd 1004 1005`
- List immediate descendants: `$ prc24s -dd 1004 1005`
- Check if a process is defunct: `$ prc24s -sz 1008 1090`

## Mandatory Requirements
Remember to execute `$killall -u username` periodically.


## Author
Meet Patel

