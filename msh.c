// The MIT License (MIT)
//
// Copyright (c) 2016 Trevor Bakker
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#define _GNU_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>

#define WHITESPACE " \t\n"      // We want to split our command line up into tokens
                                // so we need to define what delimits our tokens.
                                // In this case  white space
                                // will separate the tokens on our command line

#define MAX_COMMAND_SIZE 255    // The maximum command-line size

#define MAX_NUM_ARGUMENTS 10     // Mav shell only supports ten arguments
#define HISTORY_SIZE 15          // The maximum number of commands stored in history

char history[HISTORY_SIZE][MAX_COMMAND_SIZE];
int history_count = 0;          // Counter for the number of commands stored in history
int pid_store[15];// creating array to store the pids
int pid_counter = 0;// counter to ensure the latest pids stay in pid_store

void display_pids()
{
  for(int i=0; i< pid_counter; i++)
  {
    printf("%d. (%d)%s\n", i+1, pid_store[i], history[i]);
  }
}


void add_to_history(char *command) 
{
  if (history_count < HISTORY_SIZE) 
  {
    strcpy(history[history_count], command);
    history_count++;
  } 
  else 
  {
    // Shift all elements to the left by one
    for (int i = 0; i < HISTORY_SIZE - 1; i++) 
    {
      strcpy(history[i], history[i + 1]);
    }
    // Add the new command to the end
    strcpy(history[HISTORY_SIZE - 1], command);
  }
}

void list_history() 
{
  for (int i = 0; i < history_count; i++)
   {
    printf("%d. %s\n", i + 1, history[i]);
  }
}

int main()
{
  
  char *command_string = (char*) malloc(MAX_COMMAND_SIZE);

  while (1) {
    // Print out the msh prompt
    printf("msh> ");

    // Read the command from the commandline.  The// maximum command that will be read is MAX_COMMAND_SIZE
// This while command will wait here until the user
// inputs something since fgets returns NULL when there
// is no input
while (!fgets(command_string, MAX_COMMAND_SIZE, stdin)) {};

// Remove the newline character from the end of the string
command_string[strlen(command_string) - 1] = '\0';

if (command_string[0] == '\0' || command_string[0] == ' ') 
{
      continue;
}

int history_index = -1;
if (command_string[0] == '!') 
{
  char *history_index_str = &command_string[1];
  history_index = atoi(history_index_str);
  if (history_index > 0 && history_index <= history_count) 
  {
    strcpy(command_string, history[history_index - 1]);
  } 
  else 
  {
    printf("Error: Invalid history index\n");
    continue;
  }
}

if (history_index == -1) 
{
  // Add the command to history
  add_to_history(command_string);
}

/* Parse input */
char *token[MAX_NUM_ARGUMENTS];

for (int i = 0; i < MAX_NUM_ARGUMENTS; i++) 
{
  token[i] = NULL;
}

int token_count = 0;                                 
                                                           
// Pointer to point to the token
// parsed by strsep
char *argument_ptr = NULL;                                     
                                                           
char *working_string = strdup(command_string);
// we are going to move the working_string pointer so
// keep track of its original value so we can deallocate
// the correct amount at the end
char *head_ptr = working_string;

// Tokenize the input strings with whitespace used as the delimiter
while (((argument_ptr = strsep(&working_string, WHITESPACE)) != NULL) && 
       (token_count < MAX_NUM_ARGUMENTS)) {
  token[token_count] = strndup(argument_ptr, MAX_COMMAND_SIZE);
  if (strlen(token[token_count]) == 0) 
  {
    token[token_count] = NULL;
  }
  token_count++;
}

if (strcmp("quit", token[0]) == 0 || strcmp("exit", token[0]) == 0) 
{
  exit(0);
} 


else if (strcmp("cd", token[0]) == 0) 
{
  if (chdir(token[1]) == -1) 
  {
    printf("%s: No such file or directory.\n", token[0]);
  }
  pid_store[pid_counter] = -1;// pid is -1 when the command is cd
  pid_counter++;
} 

else if (strcmp("history", token[0]) == 0) 
{
  pid_store[pid_counter] = -1;
  pid_counter++;
  list_history();
}

else if(strcmp("history",token[0])==0 && strcmp("-p",token[1])==0)
{
    pid_store[pid_counter] = -1;// pid is -1 when the command is history
    pid_counter++;
    display_pids();
}

else 
{
  pid_t pid = fork();
  if (pid == 0) 
  {
    int r = execvp(token[0], &token[0]);
    if (r == -1) 
    {
      printf("%s: Command not found.\n", token[0]);
    }
    return 0;
  } 
  else 
  {
    pid_store[pid_counter] = pid;
    pid_counter++;
    int status;
    wait(&status);
  }
}

for (int i = 0; i < MAX_NUM_ARGUMENTS; i++) 
{
  if (token[i] != NULL) 
  {
    free(token[i]);
  }
}

free(head_ptr);
}

free(command_string);
return 0;
}