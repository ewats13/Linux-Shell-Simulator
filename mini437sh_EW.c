/************************************************************************************/
//	Eric Watson Minishell: with basic i/o redirection and piping capabilites
/***********************************************************************************/
//References:
//http://stackoverflow.com/questions/3981510/getline-check-if-line-is-whitespace
// Choppedline header file from previous iteration of ECE437
//  given permission to use by Dr. Shu
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include "chop_line.h"
#include "list.h"
#define _LINE_BUFFER 4096
#define _PROC_BUFFER 1024

list_t *proc_list;
int saved_stdout;
int saved_stdin;
int q=0;
char glb_array[10][100];

int is_empty(const char *s) {
  while (*s != '\0') {
    if (!isspace(*s))
      return 0;
    s++;
  }
  return 1;
}

//LOOK AT SPECIAL CHARACTER STRINGS!
int is_special_char(char *s){
  if(strcmp(s, "&") == 0){
    return 1;
  }
  if(strcmp(s, ">") == 0){
    return 2;
  }
  if(strcmp(s, ">>") == 0){
    return 3;
  }
  if(strcmp(s, "<") == 0){
    return 4;
  }
  if(strcmp(s, "|") == 0){
    return 5;
  }
  return 0;
}
//		CTRL-C HANDLER
void sigintHandler(int sig_num)
{   
    signal(SIGINT, sigintHandler);
    printf("\n Ctrl-C detected\n");
 //       printf("PreRun: last10\n");
 int p =0;             
    for (; p < 10; p++)
     {     
           printf("%s\n ", glb_array[p]);
     }
 
 printf("mini437sh-EW: ");
         fflush(stdout);
}

void print_last_ten()
{  
 //  printf("PreRun: last10\n");
       
   int p =0;           
    for (; p < 10; p++)
     {     
           printf("%s\n ", glb_array[p]);
     }
printf("mini437sh-EW: ");
		fflush(stdout);
}

//  Loops through all the pids in the process list. Sees if they have exited,
// if they have, then wait for them so they will be removed.

void cleanup(){
  struct list_node_t *curr_node = proc_list->head;
  int a = 0;
  int *status_ptr = &a;
  while(curr_node != NULL){
    waitpid(curr_node->val, status_ptr, WNOHANG);
    if(WIFEXITED(status_ptr)){
      waitpid(curr_node->val, status_ptr);
      list_remove_val(proc_list, curr_node->val); 
    }
    curr_node = curr_node->next;
  }
}

void final_cleanup(){
  int a = 0;
  int *status_ptr = &a;
  struct list_node_t *curr_node = proc_list->head;
  while(curr_node != NULL){
    waitpid(curr_node->val, status_ptr);
    list_remove_val(proc_list, curr_node->val);
    curr_node = curr_node->next;
  }
}

//Free command, and revert the stdout and stdin back to its original fd's
void handle_error(char **cmd){
  free(cmd);
  dup2(saved_stdout,STDOUT_FILENO);
  dup2(saved_stdin, STDIN_FILENO);
}

/*
  params
  chopped_line_t *clt - the tokenized string array of input.
  int start - where we start the parsing (not used for this part)
  char *error - pointer to a string, will be used as a reference to print a specific error
*/
int parse(chopped_line_t *clt, int start, char *error, int *pipe_num, int *fd1){
  int i;
  int is_bg = 0;
  int spec_char_val = 0;
  int cmd_cnt = 0;
  char **cmd = malloc(sizeof(*clt) * (clt->num_tokens-1));
  int stdin_flag = 0;
  int stdout_flag = 0;
  int next_pipe = 0;
  int end_pos = (clt->num_tokens);
  next_pipe = more_pipes(clt, start);
  if(next_pipe){
    end_pos = next_pipe+1;
  }
  if(strcmp(clt->tokens[(clt->num_tokens - 1)], "&") == 0){
    is_bg = 1;
  }
  for(i = start; i < end_pos; i++){
    spec_char_val = is_special_char(clt->tokens[i]);
    if(spec_char_val == 0){
      cmd[cmd_cnt] = clt->tokens[i];
      cmd_cnt++;
    }
    else{
      if(spec_char_val == 1 && i != (clt->num_tokens - 1)){
	strcpy(error, "operator & must appear at end of command line");
	handle_error(cmd);
	return -1;
      }
      if(spec_char_val == 2){
	i++;
	if(i == clt->num_tokens || is_special_char(clt->tokens[i]) > 0){
	  strcpy(error, "Missing name for redirect");
	  handle_error(cmd);
	  return -1;
	}
	if(access(clt->tokens[i], F_OK) != -1){
	  sprintf(error, "%s: File exists", clt->tokens[i]);
	  handle_error(cmd);
	  return -1;
	}
	if(stdout_flag != 0){
	  strcpy(error, "Ambiguous output redirect");
	  handle_error(cmd);
	  return -1;
	}
	int fd = open(clt->tokens[i], O_WRONLY | O_CREAT, 0777);
	dup2(fd, STDOUT_FILENO);
	close(fd);
	stdout_flag = 1;
      }
      if(spec_char_val == 3){
	i++;
	if(i == clt->num_tokens || is_special_char(clt->tokens[i]) > 0){
	  strcpy(error, "Missing name for redirect");
	  handle_error(cmd);
	  return -1;
	}
	if(stdout_flag != 0){
	  strcpy(error, "Ambiguous output redirect");
	  handle_error(cmd);
	  return -1;
	}
	int fd = open(clt->tokens[i], O_WRONLY | O_CREAT | O_APPEND, 0777);
	dup2(fd, STDOUT_FILENO);
	close(fd);
	stdout_flag = 1;
      }
      if(spec_char_val == 4){
	i++;
	if(i == clt->num_tokens || is_special_char(clt->tokens[i]) > 0){
	  strcpy(error, "Missing name for redirect");
	  handle_error(cmd);
	  return -1;
	}
	if(stdin_flag != 0){
	  strcpy(error, "Ambiguous input redirect");
	  handle_error(cmd);
	  return -1;
	}
	if(access(clt->tokens[i], F_OK) != -1){
	  int fd = open(clt->tokens[i], O_RDONLY);
	  dup2(fd, STDIN_FILENO);
	  close(fd);
	  stdin_flag = 1;
	}
	else{
	  sprintf(error, "%s: No such file or directory.", clt->tokens[i]);
	  handle_error(cmd);
	  return -1;
	}
      }
    }
  }
  int cmd_ret_val = 0;
  char *null_ptr = '\0';
  cmd[cmd_cnt] = null_ptr;
  int fd2[2];
  cmd_ret_val = exec_pipe_cmd(cmd, clt, i, pipe_num, fd1, fd2, is_bg);
  if(cmd_ret_val < 0){
    if(cmd_ret_val == -1){
      strcpy(error, "Invalid Null Command");
      handle_error(cmd);
      return -1;
    }
    if(cmd_ret_val == -2){
      strcpy(error, "execvp(): No such file or directory");
      handle_error(cmd);
      return -2;
    }
  }
  int are_more_cmds = 0;
  if(i < clt->num_tokens){
    are_more_cmds = 1;
  }
  free(cmd);
  if(are_more_cmds){
    return parse(clt, i, error, pipe_num, fd2);
  }
  return 1;
}

int exec_pipe_cmd(char **cmd, chopped_line_t *clt, int start, int *pipe_num, int *fd, int *fd2, int is_bg){
  if(cmd[0] == '\0'){
    return -1;
  }
  int are_more_cmds = 0;
  if(start < clt->num_tokens){
    are_more_cmds = 1;
  }
  if(are_more_cmds){
    pipe(fd2);
  }
  pid_t pid;
  pid = fork();
  if(pid == 0){
    if(*pipe_num > 0){
      close(fd[1]);
    }
    if(are_more_cmds){
      close(fd2[0]);
    }
    if(*pipe_num == 0 && are_more_cmds){
      dup2(fd2[1], STDOUT_FILENO);
    }
    if(*pipe_num > 0 && are_more_cmds){
      dup2(fd[0], STDIN_FILENO);
      dup2(fd2[1], STDOUT_FILENO);
    }
    if(are_more_cmds == 0 && pipe_num != 0){
      dup2(fd[0], STDIN_FILENO);
    }
    int exec_ret_val = 0;
    exec_ret_val = execvp(cmd[0], cmd);
    if(exec_ret_val < 0){
      return -2;
    }
    exit(EXIT_SUCCESS);
  }
  else{
    if(*pipe_num > 0){
      close(fd[0]);
      close(fd[1]);
    }
    if(is_bg == 0){
      int status = 0;
      waitpid(pid, &status, 0);
    }
    else{
      list_insert_val(proc_list, pid);
    }
    dup2(saved_stdout,STDOUT_FILENO);
    dup2(saved_stdin, STDIN_FILENO);
    cleanup();
    (*pipe_num)++;
    return 1;
  }
}

/*
  Returns the integer index in the chopped_line_t clt of the next pipe
  if there are no more pipes, returns 0
 */
int more_pipes(chopped_line_t *clt, int start){
  int i;
  for(i = start; i < (clt->num_tokens-1); i++){
    int spec_char_val = 0;
    if(spec_char_val = is_special_char(clt->tokens[i]) == 5){
      return i;
    }
  }
  return 0;
}

int main(void){

  signal(SIGINT, sigintHandler); //ctrl c handler
  
  char cmd[_LINE_BUFFER];
  int proc[_PROC_BUFFER];
  proc_list = list_create();
  char error_ptr[_LINE_BUFFER];

  saved_stdin = dup(STDIN_FILENO);
  saved_stdout = dup(STDOUT_FILENO);

  while(1){
    printf("mini437sh-EW: ");
    if(fgets(cmd, _LINE_BUFFER, stdin)){
/* parse command line, place tokens into array for last10 and ctrl c */
	strcpy(glb_array[q], cmd);
	q++;	

      int ret_val = 0;
      //If the string is not empty, run the code
      if(is_empty(cmd) == 0){
	chopped_line_t *clt = get_chopped_line(cmd);
	if(strcmp(clt->tokens[0], "exit") == 0){
	  free_chopped_line(clt);
	  final_cleanup(proc_list);
	  exit(EXIT_SUCCESS);
	}
	// here i can add the functionality for last 10!
	// look into passing 'history' to execvp instead
	if (strcmp(clt->tokens[0], "last10")==0){
	print_last_ten();
	}		

	int empty_fd[2];
	int pipe_0 = 0;
	int *pipe_ptr = &pipe_0;
	ret_val = parse(clt,0,error_ptr,pipe_ptr, empty_fd);
	if(ret_val < 0 ){
	  printf("%s\n", error_ptr);
	  if(ret_val == -2){
	    exit(EXIT_SUCCESS);
	  }
	}
	free_chopped_line(clt);
	dup2(saved_stdin, STDIN_FILENO);
      }
    }
    else{
      final_cleanup(proc_list);
      exit(EXIT_SUCCESS);
    }
  }
  return 0;
}
