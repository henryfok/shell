#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <dirent.h> 
#include <errno.h> 
#include <sys/stat.h>
#include <signal.h>
#include <bits/sigaction.h>

#define _BSD_SOURCE 1
#define COMMAND_LENGTH 1024
#define NUM_TOKENS (COMMAND_LENGTH / 2 + 1)
#define HISTORY_DEPTH 1000

_Bool copy = false;
char *command[1024];
int tokennum;
int status;		// Child exit status
pid_t childpid;	// Child's pid
char type_buffer[COMMAND_LENGTH];
char history[HISTORY_DEPTH][COMMAND_LENGTH];
int index = 1;
int string_length[COMMAND_LENGTH];
int error;

// Signal handler

void nothing() // Dont do anything
{

}

void add_history(int index, char *command)
{
	//index is the number assigned to the command.
	int length = (strlen(command)) + 1;
	int historysize = 1024;
	
	for (int i = 0; i < length; i++)
	{
		history[index][i] = command[i];
		string_length[index] = length;
	}
}

void display_historycopy()
{ 
	write(0, "\n", 1);
	copy = true;

	if (index != 0)
	{
		if (index < 11)
		{
			for (int i = 1; i <= index-1; i++)
			{
				printf("%d    ", i);   

				for (int j = 0; j < string_length[i]; j++)
				{
					printf("%c", history[i][j]);
				}

				printf("\n");
			}
		}

		if (index >= 11)
		{
			for (int i = index-9; i <= index; i++)
			{
				printf("%d     ", i);

				for (int j = 0; j < string_length[i]; j++)
				{
					printf("%c", history[i][j]);
				}

				printf("\n");
			}		
		}
	}
}

void display_history()
{ 
	if (index != 0)
	{
		if (index < 11)
		{
			for (int i = 1; i <= index; i++)
			{
				printf("%d    ", i);   

				for (int j = 0; j < string_length[i]; j++)
				{
					printf("%c", history[i][j]);
				}

				printf("\n");
			}
		}

		if (index >= 11)
		{
			for (int i = index-9; i <= index; i++)
			{
				printf("%d     ", i);

				for (int j = 0; j < string_length[i]; j++)
				{
					printf("%c", history[i][j]);
				}

				printf("\n");
			}		
		}
	}
}

//Tokenize command input
int tokenize_command(char *buff, char *tokens[])
{
	char *token;
	int sizearray = 1024;
	int position = 0;

	// Save command before it gets tokenized
	char *line = buff;
	command[index] = line;

	if (strlen(buff) == 0) // Dont add if blank enter pressed
	{
		return 0;
	}

	else
	{	
		add_history(index, line);	
	}

	// Get the first token
	token = strtok(buff, " \r\t\n\a");

	// Continue until all strings done
	while (token != NULL) 
	{
		tokens[position] = token;	

		position += 1;

		// Reallocate array if too full
		if (position >= sizearray)
		{
			sizearray += 1024;
			tokens = realloc(tokens, sizearray * sizeof(char *));
		}

		token = strtok(NULL, " \r\t\n\a");

		if (strcmp("exit", tokens[0]) == 0) // exit
		{
			exit(0);
		}

		if (strcmp("pwd", tokens[0]) == 0) // pwd
		{
			getcwd(tokens[0], strlen(tokens[0]));
		}

		if (strcmp("cd", tokens[0]) == 0 && tokens[1] != NULL) // cd
		{
			DIR* dir = opendir((char *)tokens[1]);

			if (dir)
			{
    			// Directory exists.
    			chdir(tokens[1]);
			}

			else if (ENOENT == errno)
			{
   				// Directory does not exist. 
				write(0, "Invalid directory", strlen("Invalid directory"));
				write(0, "\n", 1);
			}	

			closedir(dir);	
		}

		if (strcmp("history", tokens[0]) == 0)
		{
			display_history();
		}

		//////////////////////////////
		//CATCHING HISTORY COMMANDS
		//////////////////////////////

		int buff_length = strlen(buff);

		if (buff[0] == '!')
		{
			if (index == 0 || index < buff_length)
			{
				write(0, "SHELL: Unknown history command.\n", strlen("SHELL: Unknown history command.\n"));
				return 0;
			}

			if (buff[1] == '!')
			{
				// input is !!
				// run the previous command
				
				char run_command[COMMAND_LENGTH];
				
				int Prev_Length = string_length[index-1];

				for (int i = 0; i < Prev_Length; i++)
				{
					run_command[i] = history[index-1][i];
					printf("%c", run_command[i]);
				}

				printf("\n");
				system(run_command);
				
				for (int i = 0; i < Prev_Length; i++)
				{
					history[index][i] = run_command[i];
				}

				string_length[index] = Prev_Length;
			}

			else
			{
				if (index < buff_length)
				{
					write(0, "SHELL: Unknown history command.\n", strlen("SHELL: Unknown history command.\n"));
					return 0;
				}

				// input is !x
				// run the ith command
				char buff_str[buff_length-1];

				for (int i = 1; i<buff_length; i++)
				{
					buff_str[i-1] = buff[i];
				}
				int buff_int = atoi(buff_str);

				if (buff_int == 0)
				{
					// history command is invalid
					write(0, "SHELL: Unknown history command.\n", strlen("SHELL: Unknown history command.\n"));
					write(0, "\n", 1);
				}

				else
				{
					if (buff_int >= index || buff_int < (index-9))
					{
						write(0, "SHELL: Unknown history command.\n", strlen("SHELL: Unknown history command.\n"));
						write(0, "\n", 1);
					}
					
					char run_command[COMMAND_LENGTH];
				
					for (int i=0; i < string_length[buff_int]; i++)
					{
						run_command[i] = history[buff_int][i];
						printf("%c", run_command[i]);
					}

					printf("\n");
					system(run_command);

					for (int i = 0; i < string_length[buff_int]; i++)
					{
						history[index][i] = run_command[i];
					}

					string_length[index] = string_length[buff_int];
				}
			}
		}
	}

	tokennum = position;
	index++;

	return 0;
}

/**
* Read a command from the keyboard into the buffer 'buff' and tokenize it
* such that 'tokens[i]' points into 'buff' to the i'th token in the command.
* buff: Buffer allocated by the calling code. Must be at least
* COMMAND_LENGTH bytes long.
* tokens[]: Array of character pointers which point into 'buff'. Must be at
* least NUM_TOKENS long. Will strip out up to one final '&' token.
* 'tokens' will be NULL terminated.
* in_background: pointer to a boolean variable. Set to true if user entered
* an & as their last token; otherwise set to false.
*/

void read_command(char *buff, char *tokens[], _Bool *in_background)
{
	*in_background = false;

	// Read input
	int length = read(STDIN_FILENO, buff, COMMAND_LENGTH-1);

	if ( (length < 0) && (errno !=EINTR) )
	{
		perror("Unable to read command. Terminating.\n");
		exit(-1); /* terminate with error */
	}

	if ((length < 0) && (errno == EINTR))
	{
		display_historycopy();
		return;
	}

	// Null terminate and strip \n. (Adds null terminator to array)
	buff[length] = '\0';

	if (buff[strlen(buff) - 1] == '\n') 
	{
		buff[strlen(buff) - 1] = '\0';
	}

	// Tokenize (saving original command string)
	int token_count = tokenize_command(buff, tokens);

	if (token_count == 0) 
	{
		return;
	}

	// Extract if running in background:
	if (token_count > 0 && strcmp(tokens[token_count - 1], "&") == 0)
	{
		*in_background = true;
		tokens[token_count - 1] = 0;
	}
}

 /*
 * Main and Execute Commands
 */

int main(int argc, char* argv[])
{
	//Stuff for signals
	struct sigaction handler;
	handler.sa_handler = nothing;
	handler.sa_flags = 0;
	sigemptyset(&handler.sa_mask);
	sigaction(SIGINT, &handler, NULL);

	//char cwd[1024];
	char input_buffer[COMMAND_LENGTH];
	char *tokens[NUM_TOKENS];

	//int up = 35;

	// Get command
	// Use write because we need to use read()/write() to work with
	// signals, and they are incompatible with printf().
	write(STDOUT_FILENO, "> ", strlen("> "));
	_Bool in_background = false;

	while (true)
	{
		write(1, getcwd(input_buffer, sizeof(input_buffer)), 
			     strlen(getcwd(input_buffer, sizeof(input_buffer))));
		write(1, " ", 1);

		read_command(input_buffer, tokens, &in_background);
		
		childpid = fork();

		if (childpid == 0)	// Child
		{
			if (tokens[0] == NULL)  // Do nothing but newline if enter pressed
			{

			}

			if (execvp(tokens[0], tokens) == -1 && strcmp(tokens[0], "cd") != 0
			&& strcmp(tokens[0], "history") != 0 && strcmp(tokens[0], "!!") != 0
			&& copy != true)   // Print error if command is "asdf"
			{
				if (strcmp(tokens[0], "!1") != 0 && strcmp(tokens[0], "!2") != 0 && strcmp(tokens[0], "!3") != 0 && strcmp(tokens[0], "!4") != 0 && strcmp(tokens[0], "!5") != 0 && strcmp(tokens[0], "!6") != 0 && strcmp(tokens[0], "!7") != 0 && strcmp(tokens[0], "!8") != 0 && strcmp(tokens[0], "!9") != 0 && strcmp(tokens[0], "!10") != 0 && strcmp(tokens[0], "!12") != 0 && strcmp(tokens[0], "!11") != 0 && strcmp(tokens[0], "!13") != 0 && strcmp(tokens[0], "!14") != 0 && strcmp(tokens[0], "!15") != 0 && strcmp(tokens[0], "!16") != 0 && strcmp(tokens[0], "!17") != 0 && strcmp(tokens[0], "!18") != 0 && strcmp(tokens[0], "!19") != 0 && strcmp(tokens[0], "!20") != 0 && strcmp(tokens[0], "!21") != 0 && strcmp(tokens[0], "!22") != 0 && strcmp(tokens[0], "!23") != 0 && strcmp(tokens[0], "!24") != 0 && strcmp(tokens[0], "!25") != 0 && strcmp(tokens[0], "!26") != 0 && strcmp(tokens[0], "!27") != 0 && strcmp(tokens[0], "!28") != 0 && strcmp(tokens[0], "!29") != 0 && strcmp(tokens[0], "!30") != 0 && strcmp(tokens[0], "!31") != 0 && strcmp(tokens[0], "!32") != 0 && strcmp(tokens[0], "!33") != 0 && strcmp(tokens[0], "!34") != 0 && strcmp(tokens[0], "!35") != 0 && strcmp(tokens[0], "!36") != 0 && strcmp(tokens[0], "!37") != 0 && strcmp(tokens[0], "!38") != 0 && strcmp(tokens[0], "!39") != 0 && strcmp(tokens[0], "!40"))
				{
					write(0, tokens[0], strlen(tokens[0]));
					write(0, ": Unknown command.\n", strlen(": Unknown command.\n"));
				}
			}
			
			exit(0);
		}

		copy = false;

		if (in_background == false)  // Parent
		{
			waitpid(childpid, &status, WUNTRACED);
		}

		// Reset array so no conflict

		for (int i = 0; i < tokennum; i++)
		{
			tokens[i] = NULL;
		}

	}

		/*
		Steps For Basic Shell:
		1. Fork a child process
		2. Child process invokes execvp() using results in token array.
		3. If in_background is false, parent waits for
			child to finish. Otherwise, parent loops back to
			read_command() again immediately.
		*/
	
	return 0;
}
