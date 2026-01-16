#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>


#define MAX_ARGS 64
#define MAX_CMD_LEN 1024

void parse_command(char *cmd, char **args){
	int i = 0;

        //the very first cut
	char *token = strtok(cmd, " ");

	while (token != NULL) {
		//store cuts into a array
		args[i] = token;
		i++;
		//concesutivly cutting
		token = strtok(NULL, " ");
	}
	args[i] = NULL;
}
	

void read_command(char *cmd) {
	//print hints
	printf("myshell>");
	//mk sure this hint shows instantly
	fflush(stdout);

	//read lines from input 
	if (fgets(cmd,MAX_CMD_LEN,stdin) ==NULL) {
		//exit while encountering fail
		printf("\n");
		exit(0);
	}
	//clear the '\n' at the end of sentences
	cmd[strcspn(cmd,"\n")] = 0;
}

int main(){
	char cmd[MAX_CMD_LEN];
	char *args[MAX_ARGS]; // args like 'ls', '-a', NULL


	while(1){

		read_command(cmd);
		//loops until the outputs not-empty
		if (strlen(cmd) ==0) continue;
		//input 'exit' to exit the shell
		if(strcmp(cmd, "exit") ==0) break;

		parse_command(cmd, args);

		//deter if inputs are all spaces
		if (args[0] ==NULL) continue;
                
		if (strcmp(args[0], "cd") ==0) {
			
			if (args[1] == NULL) {
				printf("Expected argument to \"cd""\n");
			} else {
				//chdir overrides child process to change the places of parent
				if (chdir(args[1]) != 0) {
					perror("myshell: cd"); //dir s that doesn't exit will fail
				}
			}
			//directly forge into next loop
			continue;
		}
			
		
		//core area of multiprocessor
		// 1.call fork() to duplicate a new process
		pid_t pid = fork();

		if (pid < 0) {
			//lack of mem
			perror("Fork failed");
		}

		else if (pid == 0){
			//child process
			execvp(args[0], args);
			
                        //in case child failed
			printf("Error: Command '%s' not found.\n", cmd);
			exit(1);
		}

		else {
			//parent process
			wait(NULL);
		}


	}

	return 0;
}
