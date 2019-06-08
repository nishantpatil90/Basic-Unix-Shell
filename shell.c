#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define TOK_BUFSIZE 64
#define TOK_DELIM " \t\r\n\a"


int cd(char **args);
int help(char **args);
int n_exit(char **args);
int history();

char *builtin_str[] = {"cd","help","exit", "history"};

int (*builtin_func[]) (char **) = {
  &cd,
  &help,
  &n_exit,
  &history,
};

int num_builtins() {
  return sizeof(builtin_str) / sizeof(char *);
}

int history(){
  FILE *fph;
  char c;
  //printf("hi\n");
  fph = fopen("bash_history", "r");
  //fph = fopen("/home/nishant/.bash_history", "r");
  // bash shell stores history in .bash_history file (in case you wounder how bash stores history).
  if(fph == NULL){
    printf("Cannot open file\n");
    return (0);
  }
  c = fgetc(fph);
  while (c != EOF){
    printf("%c", c);
    c = fgetc(fph);
  }

  fclose(fph);

  return 1;

}



int cd(char **args)
{
  if (args[1] == NULL) {
    fprintf(stderr, "expected argument to \"cd\"\n");
  } else {
    if (chdir(args[1]) != 0) {
      perror("Error!");
    }
  }
  return 1;
}


int help(char **args)
{
  int i;
  printf("Nishant's Shell\n");
  printf("Type program names and arguments\n");
  printf("The following are built in:\n");

  for (i = 0; i < num_builtins(); i++) {
    printf("  %s\n", builtin_str[i]);
  }

  printf("Use the man command for information on other programs.\n");
  return 1;
}


int n_exit(char **args)
{
  return 0;
}


int launch(char **args)
{
  pid_t pid;
  int status;

  pid = fork();
  if (pid == 0) {
    // Child process
    if (execvp(args[0], args) == -1) {
      perror("execvp error");
    }
    exit(EXIT_FAILURE);
  } else if (pid < 0) {
    // Error forking
    perror("error forking");
  } else {
    // Parent process
    do {
      waitpid(pid, &status, WUNTRACED);
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
  }

  return 1;
}


int execute(char **args)
{
  int i;

  if (args[0] == NULL) {
    // An empty command was entered.
    return 1;
  }

  for (i = 0; i < num_builtins(); i++) {
    if (strcmp(args[0], builtin_str[i]) == 0) {
      return (*builtin_func[i])(args);
    }
  }

  return launch(args);
}

#define BUFSIZE 1024

char *read_line(void)
{
  int bufsize = BUFSIZE;
  int position = 0;
  char *buffer = malloc(sizeof(char) * bufsize);
  int c;

  if (!buffer) {
    fprintf(stderr, "allocation error\n");
    exit(EXIT_FAILURE);
  }

  while (1) {
    // Read a character
    c = getchar();

    if (c == EOF) {
      exit(EXIT_SUCCESS);
    } else if (c == '\n') {
      buffer[position] = '\0';
      return buffer;
    } else {
      buffer[position] = c;
    }
    position++;

    // If we have exceeded the buffer, reallocate.
    if (position >= bufsize) {
      bufsize += BUFSIZE;
      buffer = realloc(buffer, bufsize);
      if (!buffer) {
        fprintf(stderr, "allocation error\n");
        exit(EXIT_FAILURE);
      }
    }
  }
}



char **split_line(char *line)
{
  int bufsize = TOK_BUFSIZE, position = 0;
  char **tokens = malloc(bufsize * sizeof(char*));
  char *token, **tokens_backup;

  if (!tokens) {
    fprintf(stderr, "allocation error\n");
    exit(EXIT_FAILURE);
  }

  token = strtok(line, TOK_DELIM);
  while (token != NULL) {
    tokens[position] = token;
    position++;

    if (position >= bufsize) {
      bufsize += TOK_BUFSIZE;
      tokens_backup = tokens;
      tokens = realloc(tokens, bufsize * sizeof(char*));
      if (!tokens) {
    free(tokens_backup);
        fprintf(stderr, "allocation error\n");
        exit(EXIT_FAILURE);
      }
    }

    token = strtok(NULL, TOK_DELIM);
  }
  tokens[position] = NULL;
  return tokens;
}

void loop(void)
{
  char *line;
  char **args;
  int status;

  do {
    printf("> ");
    line = read_line();
    FILE *fph;
    fph = fopen("bash_history", "a");
    
    fprintf(fph,"%s", line);
    fprintf(fph,"\n");

    fclose(fph);


    args = split_line(line);
    status = execute(args);

    free(line);
    free(args);
  } while (status);
}


int main(int argc, char **argv)
{
  // Load config files, if any.

  // Run command loop.
  loop();

  // Perform any shutdown/cleanup.

  return EXIT_SUCCESS;
}
