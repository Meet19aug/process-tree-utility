#include<stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h> 
int check_pid = 0;
int flag=0;
// Function to send a specified signal to a process
void send_signal_to_process(pid_t pid, int signal) {
	int ret = kill(pid,signal); // used to send any signal to any process.
    if (ret == -1) { // error in sending signal
        perror("kill");
    } else {
        printf("Sent signal %d to process %d\n", signal, pid);
    }
}
// Function to split string based on spaces
void split_string(char *str, pid_t **pids, int *count) {
    *count = 0;
    char *ptr = str;
    char *end;
    while (*ptr) {
        long pid = strtol(ptr, &end, 10); // converts a character string to a long integer value
        if (ptr == end) {
            break; // No more numbers
        }
        (*pids)[(*count)++] = (pid_t)pid; 
        ptr = end;
    }
}
// Finction to check if pid is decendant of root or not.
int is_descendant(int root, int pid){
	// To check if process_id does belong to the process tree rooted at root_process.
	char path[40], line[100];
    FILE *status_file;
    int ppid = 0;


    snprintf(path, 40, "/proc/%d/status", pid);
    status_file = fopen(path, "r");
    if (status_file == NULL) {
        //perror("Failed to open file");
        return -1;
    }

    while (fgets(line, 100, status_file) != NULL) {
        if (strncmp(line, "PPid:", 5) == 0) {
            sscanf(line, "PPid: %d", &ppid);
            //printf("Process ID: %d, Parent Process ID: %d\n", pid, ppid);
            break;
        }
    }
    fclose(status_file);
    if (ppid == 1 ) {
        //printf("Reached the init process\n");
        return 0;
    } else if(ppid == root){
		//printf("Found the parent as root\n");
		return 1;
	} else if(ppid == 0){
		//printf("Filed in is_descendant (ppid = 0 )");
		return -1;
	}else {
        is_descendant(root, ppid);  // Recursive call with parent process ID
    }
	
}
// Function to get parent id from the processid.
int get_parent_pid(int pid) {
    char path[40], line[100];
    FILE *status_file;
    int ppid = 0;

    snprintf(path, 40, "/proc/%d/status", pid); //creating path variable pointing to status file of pid 
    status_file = fopen(path, "r");
    if (status_file == NULL) {
        perror("Failed to open file");
        return -1;
    }

    while (fgets(line, 100, status_file) != NULL) {
        if (strncmp(line, "PPid:", 5) == 0) {
            sscanf(line, "PPid: %d", &ppid); // extracting ppid from file .
            //printf("Process ID: %d, Parent Process ID: %d\n", pid, ppid);
            break;
        }
    }
    fclose(status_file);
    //printf("%d\n",ppid);
	return ppid;
}
// Function to extract the status of process from /proc/pid/status file.
char* extract_status(int pid) {
	if(pid<1){
		return NULL;
	}
    char path[40], line[100];
    FILE *status_file; 
    char *process_stat;

    snprintf(path, 40, "/proc/%d/status", pid);// creating path variable to point status to pid.
    status_file = fopen(path, "r");
    if (status_file == NULL) {
        perror("Failed to open file");
        return NULL;
    }

    while (fgets(line, 100, status_file) != NULL) {
        if (strncmp(line, "State:", 6) == 0) {
			process_stat = strdup(line + 7);
            //printf("Process ID: %d, Parent Process ID: %d\n", pid, ppid);
            break;
        }
    }
    fclose(status_file);
    //printf("%d\n",ppid);
	return process_stat;
}
// generalize function for different options.
void signal_all_children(pid_t pid, int signal,int include_direct_decendant, int include_nondirect_decendant,int list_pid,int list_siblings_only,char * process_stat) {
    char path[256];
    snprintf(path, sizeof(path), "/proc/%d/task/%d/children", pid, pid); // creating the path variable which help in listing the child process.

    FILE *fp = fopen(path, "r");
	// check for standard file io problems.
    if (fp == NULL) {
        perror("fopen");
        return;
    }

    char line[1024];
	// reading file line by line.
    if (fgets(line, sizeof(line), fp) != NULL) {
        int count = 0;
		// list of child pids stored in array of pid_t. 
        pid_t *child_pids = (pid_t *)malloc(1024 * sizeof(pid_t));
        split_string(line, &child_pids, &count);
		// works when list_sibling_only flag is 1.
		if(list_siblings_only){
			while(count>0){// iterating through all the children
				if(child_pids[count]!=check_pid){ // checking it with global varaible 
					if(child_pids[count]==0){ // exceptional condition..
						count--;
						continue;
					}
					if(process_stat!=NULL){
						char * found_status_of_process = extract_status(child_pids[count]); // extracting status for the pid using extract_status function.
						if(found_status_of_process!=NULL){ 
							int matched = strcmp(found_status_of_process,process_stat); // comparing the extracted process from the user passed argument process.
							if(matched == 0){
								printf("%d\n",child_pids[count]);
								flag=1;
							}
						}
					}else{
						printf("%d\n",child_pids[count]);// printing child pid.
						flag=1;
					}
				}
				count--;
			}
			return;
		}
        for (int i = 0; i < count; i++) {
            signal_all_children(child_pids[i], signal,include_direct_decendant,include_nondirect_decendant,list_pid,list_siblings_only,process_stat);
			if(!list_pid){ // sending signal work happen here
				if(process_stat!=NULL){
					char * found_status_of_process = extract_status(child_pids[i]); //extracting status.
					if(found_status_of_process!=NULL){
						int matched = strcmp(found_status_of_process,process_stat);// comparing process with user passeed argument..
						if(matched == 0){
							int parent_id = get_parent_pid(child_pids[i]);// finding parent porcess id and passing recursively inro next iteration.
							send_signal_to_process(parent_id,signal);
						}
					}
				}else{
					send_signal_to_process(child_pids[i], signal);
				}
			}else{ // listing works happen here 
				int parent_id = get_parent_pid(child_pids[i]);
				if(parent_id==-1){
					printf("Failed due to fp\n");
				}
				if(process_stat!=NULL){
					//printf("process_stat is not null %d\n",child_pids[i]);
					char * found_status_of_process = extract_status(child_pids[i]);
					if(found_status_of_process!=NULL){
						int matched = strcmp(found_status_of_process,process_stat);
						if(matched == 0){
							flag=1;
							printf("%d\n",child_pids[i]);
						}
					}
				}else{
					if(parent_id==check_pid && include_direct_decendant){ // current child is immediate decendent of process_id[check_process]
						printf("%d\n",child_pids[i]);
						flag=1;
					}else if(parent_id!=check_pid && include_nondirect_decendant){ // current child is non direct decendent of the given processid
						printf("%d\n",child_pids[i]);
						flag=1;
					}
				}
			}
        }
        free(child_pids);
    }
    fclose(fp);
}

void print_grand_child(int process_id){
	char path[256];
    snprintf(path, sizeof(path), "/proc/%d/task/%d/children", process_id, process_id);

    FILE *fp = fopen(path, "r");
    if (fp == NULL) {
        perror("fopen");
        return;
    }

    char line[1024];
	if (fgets(line, sizeof(line), fp) != NULL) {
        int count = 0;
		int p1=0;
		int p2=0;
        pid_t *child_pids = (pid_t *)malloc(1024 * sizeof(pid_t));
        split_string(line, &child_pids, &count); // function to extract count and list of children process.
		for (int i = 0; i < count; i++) {
			//printf("Child PId : %d",child_pids[i]);
			print_grand_child(child_pids[i]);
			p1=get_parent_pid(child_pids[i]);// finding parent of the pid,
			p2=get_parent_pid(p1); // finding grand parent from passing p1 as father.
			if(p2==check_pid){
				flag=1;
				printf("%d\n",child_pids[i]);
			}
		}
	}
}
//The root_process kills all its descendants using SIGKILL
void signaldx(int root_process, int pid){
	if(is_descendant(root_process, pid)==1){
		signal_all_children(root_process,9,1,1,0,0,NULL);
	}else{
		printf("The process %d does not belong to the tree rooted at %d.\n", pid,root_process);
	}
}
//The root_process sends SIGSTOP to all its descendants
void signaldt(int root_process, int pid){
	if(is_descendant(root_process, pid)==1){
		signal_all_children(root_process,19,1,1,0,0,NULL);
	}else{
		printf("The process %d does not belong to the tree rooted at %d.\n", pid,root_process);
	}
}
//The root_process sends SIGCONT to all its descendants that have been paused
void signaldc(int root_process, int pid){
	if(is_descendant(root_process, pid)==1){
		signal_all_children(root_process,18,1,1,0,0,NULL);
		printf("The process %d does not belong to the tree rooted at %d.\n", pid,root_process);
	}
}
//root_process kills process_id
void signalrp(int root_process,int process_id){
	if(is_descendant(root_process, process_id)==1){
		send_signal_to_process(process_id,9);
	}else{
		printf("The process %d does not belong to the tree rooted at %d.\n", process_id,root_process);
	}
}
//lists the PIDs of all the non-direct descendants of process_id
void signalnd(int root_process,int process_id){
	if(is_descendant(root_process, process_id)==1){
		check_pid=process_id;
		signal_all_children(process_id,9,0,1,1,0,NULL);
		if(flag==0){
			printf("No -  non direct decendants\n");
		}
	}else{
		printf("The process %d does not belong to the tree rooted at %d.\n", process_id,root_process);
	}
}
//lists the PIDs of all the immediate descendants of process_id
void signaldd(int root_process,int process_id){
	if(is_descendant(root_process, process_id)==1){
		check_pid=process_id;
		signal_all_children(process_id,9,1,0,1,0,NULL);
		if(flag==0){
			printf("No -  direct decendants\n");
		}
	}else{
		printf("The process %d does not belong to the tree rooted at %d\n", process_id,root_process);
	}
}
//lists the PIDs of all the sibling processes of process_id
void signalsb(int root_process,int process_id){
	if(is_descendant(root_process, process_id)==1){
		int res = get_parent_pid(process_id);
		if(res<1){
			printf("Failed due to fp\n");
		}else{
			check_pid=process_id;
			signal_all_children(res,9,1,0,1,1,NULL);
			if(flag==0){
				printf("No - sibling of process id.\n");
			}
		}
	}else{
		printf("The process %d does not belong to the tree rooted at %d\n", process_id,root_process);
	}
}
//lists the PIDs of all the sibling processes of process_id that are defunct
void signalbz(int root_process,int process_id){
	if(is_descendant(root_process, process_id)==1){
		int res = get_parent_pid(process_id);
		if(res<1){
			printf("Failed due to fp\n");
		}else{
			signal_all_children(res,9,1,0,1,1,"Z (zombie)\n");
			if(flag==0){
				printf("No - sibling of process id that are defunct.\n");
			}
		}
	}else{
		printf("The process %d does not belong to the tree rooted at %d\n", process_id,root_process);
	}
}
//Lists the PIDs of all descendents of process_id that are defunct
void signalzd(int root_process,int process_id){
	if(is_descendant(root_process, process_id)==1){
		int res = get_parent_pid(process_id);
		if(res<1){
			printf("Failed due to fp\n");
		}else{
			signal_all_children(res,9,1,1,1,0,"Z (zombie)\n");
			if(flag==0){
				printf("No - decendants of process_id that are defunct.\n");
			}
		}
	}else{
		printf("The process %d does not belong to the tree rooted at %d\n", process_id,root_process);
	}
}
//lists the PIDs of all the grandchildren of process_id
void signalgc(int root_process,int process_id){
	if(is_descendant(root_process, process_id)==1){
		check_pid=process_id;
		print_grand_child(process_id);
		if(flag==0){
				printf("No - grandchildren.\n");
		}
	}else{
		printf("The process %d does not belong to the tree rooted at %d\n", process_id,root_process);
	}
}
//prints the status of the process_id (Defunct/ Not Defunct)
void signalsz(int root_process,int process_id){
	if(is_descendant(root_process, process_id)==1){
		char *found_status = extract_status(process_id);
		if(strcmp(found_status,"Z (zombie)\n")==0){
			printf("Process is Defunct\n");
		}else{
			printf("Process is Not Defunct\n");
		}
	}else{
		printf("The process %d does not belong to the tree rooted at %d\n", process_id,root_process);
	}
}
//Kills the parents of all zombie process that are the descendants of proceed_id
void signalkz(int root_process,int process_id){
	if(is_descendant(root_process, process_id)==1){
		check_pid=process_id;
		signal_all_children(process_id,9,1,1,0,0,"Z (zombie)\n");
	}else{
		printf("The process %d does not belong to the tree rooted at %d\n", process_id,root_process);
	}
}

int main(int argc, char *argv[]){
	if (argc < 3 || argc > 4) {
       return 1;
    }

	pid_t root_process = atoi(argv[argc - 2]);
    pid_t process_id = atoi(argv[argc - 1]);

	if(root_process<=0 || process_id<=0){
		printf("Process Id can't be zero or negative\n");
		return 1;
	}else if(process_id == 1 ){
		printf("You need special permission to work with init process\n");
		return 1;
	}

	//printf("Pid of root is : %d", root_process);
	//printf("Pid of process is : %d",process_id);

	if(argc==3){// no option is provided...
		if(is_descendant(root_process, process_id)==1){
			//printf("The process %d belongs to the tree rooted at %d\n", process_id, root_process);
			printf("%d\n",process_id);
			int res = get_parent_pid(process_id);
			if(res==-1){
				perror("fopen");
			}else{
				printf("%d\n",res);
			}
		}
		else{
			printf("The process %d does not belong to the tree rooted at %d.\n", process_id,root_process);
		}
		//else does not print anything...
	}else{ // option is provided....
		char* option = argv[argc-3];
		if (strcmp("-dx", option) == 0) {
			signaldx(root_process, process_id);
        	//printf("The root_process kills all its descendants using SIGKILL\n");
    	} else if (strcmp("-dt", option) == 0) {
			signaldt(root_process, process_id);
        	//printf("The root_process sends SIGSTOP to all its descendants\n");
		} else if (strcmp("-dc", option) == 0) {
			signaldc(root_process, process_id);
			//printf("The root_process sends SIGCONT to all its descendants that have been paused\n");
		} else if (strcmp("-rp", option) == 0) {
			signalrp(root_process,process_id);
			//printf("Root_process kills process_id\n");
		} else if (strcmp("-nd", option) == 0) {
			signalnd(root_process,process_id);
			//printf("Lists the PIDs of all the non-direct descendants of process_id\n");
		} else if (strcmp("-dd", option) == 0) {
			signaldd(root_process,process_id);
			//printf("Lists the PIDs of all the immediate descendants of process_id\n");
		} else if (strcmp("-sb", option) == 0) {
			signalsb(root_process,process_id);
			//printf("Lists the PIDs of all the sibling processes of process_id\n");
		} else if (strcmp("-bz", option) == 0) {
			signalbz(root_process,process_id);
			//printf("Lists the PIDs of all the sibling processes of process_id that are defunct\n");
		} else if (strcmp("-zd", option) == 0) {
			signalzd(root_process,process_id);
			//printf("Lists the PIDs of all descendants of process_id that are defunct\n");
		} else if (strcmp("-od", option) == 0) {
			printf("Lists the PIDs of all descendants of process_id that are orphans\n");
		} else if (strcmp("-gc", option) == 0) {
			signalgc(root_process,process_id);
			//printf("Lists the PIDs of all the grandchildren of process_id\n");
		} else if (strcmp("-sz", option) == 0) {
			signalsz(root_process,process_id);
			//printf("Prints the status of the process_id (Defunct/Not Defunct)\n");
		} else if (strcmp("-so", option) == 0) {
			printf("Prints the status of the process_id (Orphan/Not Orphan)\n");
		} else if (strcmp("-kz", option) == 0) {
			signalkz(root_process,process_id);
			//printf("Kills the parents of all zombie processes that are the descendants of process_id\n");
		} else {
			printf("syntax: prc24s [-options] root_id process_id \n");
		}
	}
	return 0;
}

