/*
 * Author:  David Bethala
 * Date:  08 October 2014
 * Class:  CIS3207 - Intro to Systems Programming and Operating Systems
 * Abstract:  This program is intended to duplicate the UNIX shell in its most basic of functions, creating and launching processes.
 */

#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>

#define BUFLEN 1024

int default_launch_process(char*, char**);
char** my_parser(char*);
char** my_parser_2(char**);
int arg_counter(char**);
char* mode_setter(char**);
void redirect_launch_process(char**, char**, char*);
void pipeline_launch_process(char**, char**);


/*Function my_parser accepts a string as input and returns an array of strings as output*/
char** my_parser(char* instr){
	
	//Array to be returned to the caller
	char** retstr = (char**)malloc(BUFLEN*sizeof(char*));
	
	//String used to hold each word of the string, tokenized
	char* token = (char*)malloc(sizeof(char));

	//Copy of the string made from strcpy
	char* copied_string = (char*)malloc(strlen(instr));
	
	//Array counter
	int i = 0;
	
	//strcpy function used to copy input string to a local variable
	strcpy(copied_string, instr);
	
	//strtok used to separate the input string by using whitespace as a delimiter
	token = strtok(copied_string, " ");
	
	//The token is placed into the array
	retstr[i] = token;

	//Array counter incremented
	i++;
	
	//While loop iterates until NULL termination of the local string. Each token is placed into the array before the array is iterated.
	while(token != NULL){
		token = strtok(NULL, " ");
		retstr[i] = token;
		i++;
	}
	
	//Null termination of the return string
	retstr[i] = NULL;
	
	return retstr;
}

/*Function my_parser_2 takes the array created by my_parser and decrements the position of each element in the array*/
char** my_parser_2(char** inarr){
	
	//Initializing the return array of strings
	char** retstr = (char**)malloc(1024*(sizeof(char*)));
	
	//Index value
	int index = 1;
	
	//Loop iterates until NULL is encountered. Each element in the input array is moved one position up. Old_array[1] == new_array[0].
	while(inarr[index] != NULL){
		retstr[index-1] = inarr[index];
		index++;	
	}
	//NULL-terminating the new array
	retstr[index-1] = NULL;

	return retstr; 

}

/*Function argcounter accepts an array of strings and returns the total number elements in the array.*/	
int arg_counter(char** inarr){
	
	//Value to be returned
	int counter = 0;
	
	//While loop iterates untl the array is Null terminated. The counter is incremented after each pass.
	while(inarr[counter] != NULL){
		counter++;
	}
	return counter;
}

/*Fucntion accepts the argument vector as input and returns a string describing the nature of input/output and backgrounding needed for process creation and management*/
char* mode_setter(char** in_arr){
	
	//Initializing the return string which will hold the mode of input/output
	char* mode = (char*)malloc(30*sizeof(char));	
	
	//Variable to hold each array  element
	char* token;	

	//Array index positions
	int index = 0;
	
	//Default mode
	mode = "NORMAL";

	//Loop iterates while NULL has not been encountered or a specified symbol is encountered
	while(in_arr[index] != NULL){
		token = in_arr[index];
		
		//If-else statements to evaluate each scenario
		//Handling input redirection
		if(strcmp(token, "<") == 0){
			mode = "INPUT_REDIRECT";
			break;
		}
		//Handling output redirection
		else if(strcmp(token, ">") == 0){
			mode = "OUTPUT_REDIRECT";
			break;
		}
		//Handling piping
		else if(strcmp(token, "|") == 0){
			mode = "PIPELINE";
			break;
		}
		//Handling backgrouding
		else if (strcmp(token, "&") == 0){
			mode  = "BACKGROUND";
			break;
		}
		//Iterate if none of the above
		else{
			index++;	
		}
	}
	return mode;
}

/*Function launch_process acceots the name of a program and its arguments as parameters. If able, it forks the process and executes the commands*/ 
int default_launch_process(char* program_name, char** argument_vector){

	
	//Variable used store process ID
	pid_t my_pid;
	
	//Integer value to store the status of the chlld process
	int* status;
	
	//Obtain the mode of the process
	char* my_mode = mode_setter(argument_vector);
	
	//Return value for fork()
	my_pid = fork();
	
	//Error to handle process error
	if(my_pid < 0){	
		//Print error message upon fork failing
		printf("Error forking the process.\n");
		//Exit the process
		exit(1);
	}
	
	//Parent process
	else if(my_pid > 0){

		//If background process, wait until child returns
		if(strcmp(my_mode, "BACKGROUND") == 0){
			wait(&status);
		}
	}
	
	//Child process
	else{
		//Execute the program
		if(execvp(program_name, argument_vector) < 0){
			//Error upon execvp failure
			printf("Error. execvp failed to execute.\n");
			//Exit the process
			exit(1);
		}
		
	}
}

/*Function that handles processes which utilize I/O Redirection.  It accepts the command name, the arguments to be handled before redirection and the arguments to be handled after.*/
void redirect_launch_process(char** args_vector_1, char** args_vector_2, char* mode){
	
	//Variable used to hold the Process ID return
	pid_t pid;
	
	int in;
	int out;
	
	//Status of child process
	int status;
	
	//Forking the process
	pid = fork();
	
	//Error check
	if (pid < 0){
		printf("Error forking\n");
		exit(1);
	}
	
	//On successful fork
	else if(pid == 0){
	
		//For output redirection
			if(strcmp(mode, "OUPUT_REDIRECT") == 0){
				out = open(args_vector_2[0], O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
				dup2(out, 1);
				close(out);
		
			
			}
	
		//For input redirection
			else if(strcmp(mode, "INPUT_REDIRECT") == 0){
				in  = open(args_vector_2[0], O_RDONLY);
				dup2(in, 0);
				close(in);
				
			}
			
		//Execute the commands
		execvp(args_vector_1[0], args_vector_1);
		printf("Execvp failed\n");
		exit(1);
	}
	
	//Parent process
	else{
	
		wait(&status);	

	}
}

/*Function pipeline_launch_process accepts two string arrays as input and creates a pipeline of communication between the two processes*/
void pipeline_launch_process(char** in_arr_1, char** in_arr_2){
	
	//Process ID variables
	pid_t pid;
	
	//Declaring the file descriptor array
	int fds[2];
	
	//Create the pipe
	pipe(fds);
	
	if(pid == 0){
	
		dup2(fds[0], 0);
	
		close(fds[1]);
		
		execvp(in_arr_2[0], in_arr_2);
		printf("Error executing the commands\n");
		exit(1);
	}
	
	else{	
		
		dup2(fds[1], 1);
	
		close(fds[0]);
	
		execvp(in_arr_1[0], in_arr_1);
		printf("Error executing the commands\n");
		exit(1);	
	
	}
}
int main(int argc, char** argv){

	//Initializing the main argument vector
	char** args_vector = (char**)malloc(BUFLEN*sizeof(char*));
	
	//Initializing the input buffer
	char* input_buffer = (char*)malloc(BUFLEN*sizeof(char));
	
	//Secondary argument vector for I/Redirection
	char** args_vector_2 = (char**)malloc(BUFLEN*sizeof(char*));
	
	//Name of the command to be executed
	char* program_name;
	
	//Mode of the input/output
	char* mode;
	
	//Total number of arguments (command not included)
	int arg_count = 0;
	
	//Token used to store individual array elements
	char* token;
	
	//Position of trailing newline character
	char* pos;
	
	//Loop iterates until "exit" is entered
	while(1){	
		
		//Shell prompt
		printf("my_shell>: ");
		
		//Readint a line from STDIN
		fgets(input_buffer, BUFLEN, stdin);
		
		//Parsing the arguments
		args_vector = my_parser(input_buffer);
		
		//Separating the name of the command from the rest of the arguments
		program_name = args_vector[0];
		
		//Total number of arguments ocunted
		arg_count = arg_counter(args_vector);
	
		//Storing the final argument into a variable	
		pos = args_vector[arg_count-1];
	
		//Accessing the final character of the string and replacing newline with null
		pos[strlen(pos)-1] = '\0';
		
		//Placing the string back into the argument vector
		args_vector[arg_count-1] = pos;
		
		//Checking exit condiiton
		if(strcmp(program_name, "exit") == 0){
			exit(1);
			return 0;
		}
		
		//Checking the mode for I/O
		mode = mode_setter(args_vector);
		
		//Initializing the index
		int index = 0;
		
		//Initializing the index for the new array
		int newidx = 0;
		
		//IF I/O Redirection is needed
		if((strcmp(mode, "INPUT_REDIRECT") == 0) || (strcmp(mode, "OUTPUT_REDIRECT") == 0) || (strcmp(mode, "PIPELINE") == 0)){
			
			//Loop iterates until the array is NULL-terminated
			while(args_vector[index] != NULL){
				
				//Each element is stored in its variable
				token = args_vector[index];
			
				//If the redirection token is located
				if((strcmp("<", token) == 0) || (strcmp(">", token) == 0) || (strcmp("|", token) == 0)){
				
					//The element is replaced by NULL
					args_vector[index] = NULL;
					
					//The index is incremented
					index++;
			
					//Inner loop iterates until the first array is NULL terminated
					while(args_vector[index] != NULL){
						
						//Each remaining element is stored in the new array
						args_vector_2[newidx] = args_vector[index];
						
						//Incrementing the indices
						newidx++; index++;
					}
						//NULL terminatng the new array
						args_vector_2[newidx] = NULL;
						
						if(strcmp(mode, "PIPELINE") == 0){
						
							pipeline_launch_process(args_vector, args_vector_2);
							break;
						}
						else if((strcmp(mode, "INPUT_REDIRECT") == 0) || (strcmp(mode, "OUTPUT_REDIRECT") == 0)){

							//Separate launch process for redirection
							redirect_launch_process(args_vector, args_vector_2,  mode);
							break;
						}
				}
				else{
					//Incrment the array index
					index++;
				}
			}
		}
		//If a normal process
		else if (strcmp(mode, "NORMAL") == 0){	
			default_launch_process(program_name, args_vector);	
		}
	}		
	free(args_vector);
	free(input_buffer);	
	return 0;
}

