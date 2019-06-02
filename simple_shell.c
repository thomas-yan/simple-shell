/***************************************************
 *
 * @author       Linwish
 *
 * @date         20190601
 *
 * @discription  A simple shell written in c
 *
 ****************************************************/

#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1
#define TOKEN_DELIMITER " \t\r\n\a"
#define TOKEN_BUFFSIZE 128
#define READLINE_BUFFSIZE 2048

// builtin cmds declaration
int shell_cd(char **input_args);
int shell_help(char **input_args);
int shell_exit(char **input_args);
int shell_sheep(char **input_args);

// builtin cmd string
char *builtin_str[] = {
  "cd",
  "help",
  "exit",
  "sheep"
};

// builtin cmd list
int (*builtin_func[]) (char **) = {
  &shell_cd,
  &shell_help,
  &shell_exit,
  &shell_sheep
};

// builtin cmd count
int builtins_num() {
  return sizeof(builtin_str) / sizeof(char *);
}

// Builtin cmd sheep

int shell_sheep(char **input_args) {
  char *ascii_str = "\n"
"                                          ,ooooooooooo,              \n"
"                                       ,;OOOOOOOOOOOOOOo,            \n"
"                                    ,ooOOOOOOOOOOOOOOOOOo;,,,,       \n"
"                                ,ooooOOOOOOOOOOOOOOOOOOOOooo(@`,     \n"
"                     _  __   __;oooooo OOOOOOOOOOO; ;@@@@@@@o;@@`,   \n"
"            _______/@@@@@@@@@)ooOOOOOO) oOOOOOOOOOo; ;o@@@@@o;@@@:   \n"
"           /######)@@@@@@@@@@( _______ ( oOOOOOOOOOOo o@@@@@@`@@@`;  \n"
"          <######)@@@@@@@@@@@@(######/  `,;;;  oOOOOOo @@@@@@o, @@;  \n"
"               ` @@@@@@@@@@@@(######/ oO (@@@@  oOOOOO;@@@@@@@,  @`, \n"
"                ))@@@@@@@@@@(       oOOOo@@@@@@: ;;oo,`o@@@@@;   (@) \n"
"                )  `@@@@@@@( (  ooOOOOOoo:@@@@@: ,' /###o@@@@;       \n"
"                ( (0)     (0) ) ooooooo /@@@@@/,`  :####o@@@@;       \n"
"                 )           ( `'`'`'`'/@@@@@/     /###/:@@@@;       \n"
"                  `,       ,'     /###/@@@@@/     :###; :@@@@@;      \n"
"                     _`_'_/      /###/@@@@/       :###; :@@@@@;      \n"
"                     ~~~~~      ;###;@@@@/        :##;  `:@@@@;      \n"
"                                ;###;@@@@;       /  /    :@@@;       \n"
"                               /~~~~;@@@@;      `-^-'    /   /       \n"
"                               `-^--;@@@@/               `-^--'      \n"
"                                    :~~~~:                           \n"
"                                     /__/                            \n";
  puts(ascii_str);
  return EXIT_SUCCESS;
}

// Builtin cmd cd
int shell_cd(char **input_args) {
  if (input_args[1] == NULL) {
    fprintf(stderr, "SimpleSH: expected argument to \"cd\"\n");
  } else {
    if (chdir(input_args[1]) != 0) {
      perror("lsh");
    }
  }
  return 1;
}

// Builtin cmd help
int shell_help(char **input_args) {
  int i;
  printf("Simple Shell\n");
  printf("Type the command you like, and hit enter\n");
  printf("The following are built in cmds:\n\n");

  for (i = 0; i < builtins_num(); i++) {
    printf("  %s\n", builtin_str[i]);
  }

  printf("\nUse the man command for information on other programs.\n");
  return EXIT_SUCCESS;
}

// Builtin cmd exit
int shell_exit(char **input_args) {
  return EXIT_SUCCESS;
}

// Read input
char *read_line(void) {
  int buff_size = READLINE_BUFFSIZE;
  int pos = 0;
  char *buffer = malloc(sizeof(char) * buff_size);
  int c;

  if (!buffer) {
    fprintf(stderr, "SImpleSH: allocation failure!\n");
    exit(EXIT_FAILURE);
  }

  while (1) {
    c = getchar();
    if (c == EOF) {
      exit(EXIT_SUCCESS);
    } 
    else if (c == '\n') {
      buffer[pos] = '\0';
      return buffer;
    } 
    else {
      buffer[pos] = c;
    }
    pos++;

    if (pos >= buff_size) {
      buff_size += READLINE_BUFFSIZE;
      buffer = realloc(buffer, buff_size);
      if (!buffer) {
        fprintf(stderr, "SImpleSH: allocation failure!\n");
        exit(EXIT_FAILURE);
      }
    }
  }
}

// Split input to tokens
char **split_line(char *input_line) {
  int buff_size = TOKEN_BUFFSIZE, pos = 0;
  char **tokens = malloc(buff_size * sizeof(char*));
  char *token, **tokens_backup;

  if (!tokens) {
    fprintf(stderr, "SimpleSH: allocation failure!\n");
    exit(EXIT_FAILURE);
  }

  token = strtok(input_line, TOKEN_DELIMITER);
  while (token != NULL) {
    tokens[pos] = token;
    pos++;

    if (pos >= buff_size) {
      buff_size += TOKEN_BUFFSIZE;
      tokens_backup = tokens;
      tokens = realloc(tokens, buff_size * sizeof(char*));
      if (!tokens) {
		    free(tokens_backup);
        fprintf(stderr, "SImpleSH: allocation failure!\n");
        exit(EXIT_FAILURE);
      }
    }

    token = strtok(NULL, TOKEN_DELIMITER);
  }
  tokens[pos] = NULL;
  return tokens;
}

// Fork a child to execute
int shell_launch(char **input_args) {
  pid_t pid;
  int status;

  pid = fork();
  if (pid == 0) {
    if (execvp(input_args[0], input_args) == -1) {
      perror("SimpleSH: Error executing cmd.");
    }
    exit(EXIT_FAILURE);
  } 
  else if (pid < 0) {
    perror("SimpleSH: Error forking");
  } 
  else {
    do {
      waitpid(pid, &status, WUNTRACED);
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
  }
  return EXIT_FAILURE;
}

// Execute cmd
int shell_exec(char **input_args) {
  if (input_args[0] == NULL) {
    return EXIT_FAILURE;
  }

  for (int i = 0; i < builtins_num(); i++) {
    if (strcmp(input_args[0], builtin_str[i]) == 0) {
      return (*builtin_func[i])(input_args);
    }
  }

  return shell_launch(input_args);
}

// REPL loop
void shell_loop(void) {
  char *input_line;
  char **input_args;
  int status;

  while(1) {
    printf("> ");
    input_line = read_line();
    input_args = split_line(input_line);
    status = shell_exec(input_args);

    free(input_line);
    free(input_args);

    if (!status) break;
  }
}

// The main entry
int main(int argc, char **argv) {
  shell_loop();
  return EXIT_SUCCESS;
}