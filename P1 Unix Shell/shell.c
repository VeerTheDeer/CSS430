// ----------------------------------- shell.c -----------------------------------
// Author: Dhruva Pyapali and Veer Saini
// Course: CSS 430
// Creation Date: Apr 10, 2025
// Last Modified: Apr 13, 2025
// ---------------------------------------------------------------------------------
// Purpose:
// This file implements the UNIX-like shell interface.
// It provides constants, data structures, and function prototypes to support:
// - Interactive command execution with background processes (&)
// - Input/output redirection (< and >)
// - Command piping (|)
// - Command history (!!)
// - Built-in test cases
// - ASCII art Easter egg
// - Error handling for invalid commands
// ---------------------------------------------------------------------------------

#include "shell.h"

// ----------------------- int main(int argc, char **argv) ----------------------
// Purpose:
// - Launches interactive mode if --interactive flag is written,
//   otherwise runs built-in test cases.
// Parameters:
// - argc - num of command-line arguments
// - argv - array of command-line argument strings
// ------------------------------------------------------------------------------
int main(int argc, char **argv) {
  if (argc == 2 && equal(argv[1], "--interactive")) {
    return interactiveShell();
  } else {
    return runTests();
  }
}

// --------------------------- void print_ascii_art() ---------------------------
// Purpose:
// - Displays ASCII art for the "ascii" command.
// ------------------------------------------------------------------------------
void print_ascii_art() {
  printf("  |\\_/|        ****************************    (\\_/)\n");
  printf(" / @ @ \\       *  \"Purrrfectly pleasant\"  *   (='.'=)\n");
  printf("( > º < )      *       Poppy Prinz        *   (\")_(\")\n");
  printf(" `>>x<<´       *   (pprinz@example.com)   *\n");
  printf(" /  O  \\       ****************************\n");
}

// ---------- execute_single_command(char **tokens, bool background) ------------
// Purpose:
// - Handles input/output redirection using dup2()
// - Forks child process for command execution
// - Parent waits for foreground commands
// - Special handling for "ascii" command
// Parameters:
// - tokens - array of command-line arguments
// - background - boolean flag for background execution
// ------------------------------------------------------------------------------
void execute_single_command(char **tokens, bool background) {
  if (tokens[0] == NULL) return;

  // Ascii art command handling
  if (equal(tokens[0], "ascii") && tokens[1] == NULL) {
    print_ascii_art();
    return;
  }

  // Check for input/output redirection
  char *input_file = NULL;
  char *output_file = NULL;
  int input_fd = -1, output_fd = -1;

  for (int i = 0; tokens[i] != NULL; i++) {
    if (equal(tokens[i], "<")) {
      input_file = tokens[i + 1];
      tokens[i] = NULL;
      break;
    } else if (equal(tokens[i], ">")) {
      output_file = tokens[i + 1];
      tokens[i] = NULL;
      break;
    }
  }

  // Check for background execution
  pid_t pid = fork();
  if (pid == 0) {

    // Handle input/output redirection
    if (input_file) {
      input_fd = open(input_file, O_RDONLY);
      if (input_fd == -1) {
        perror("open");
        exit(EXIT_FAILURE);
      }
      dup2(input_fd, STDIN_FILENO);
      close(input_fd);
    }

    if (output_file) {
      output_fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
      if (output_fd == -1) {
        perror("open");
        exit(EXIT_FAILURE);
      }
      dup2(output_fd, STDOUT_FILENO);
      close(output_fd);
    }

    // Execute commnad
    execvp(tokens[0], tokens);
    perror("execvp");
    exit(EXIT_FAILURE);
  } 
  else if (pid > 0 && !background) {waitpid(pid, NULL, 0);}
  else if (pid < 0) {perror("fork");}
}

// ------- execute_pipeline(char **left, char **right, bool background) --------
// Purpose:
// - Creates pipe and connects commands via dup2
// - Forks processes for both sides of pipe
// - Manages file descriptor cleanup
// Parameters:
// - left - array of tokens for left command (before pipe)
// - right - array of tokens for right command (after pipe)
// - background - boolean flag for background execution
// ------------------------------------------------------------------------------
void execute_pipeline(char **left, char **right, bool background) {
  int pipefd[2];
  if (pipe(pipefd) == -1) {
    perror("pipe");
    return;
  }

  // Fork processes for left and right commands

  // Fork process for left command
  pid_t left_pid = fork();
  if (left_pid == 0) {
    close(pipefd[RD]);
    dup2(pipefd[WR], STDOUT_FILENO);
    close(pipefd[WR]);
    execute_single_command(left, false);
    exit(EXIT_SUCCESS);
  }
   
  // Fork process for right command
  pid_t right_pid = fork();
  if (right_pid == 0) {
    close(pipefd[WR]);
    dup2(pipefd[RD], STDIN_FILENO);
    close(pipefd[RD]);
    execute_single_command(right, false);
    exit(EXIT_SUCCESS);
  }

  // Cleanup
  close(pipefd[RD]);
  close(pipefd[WR]);

  if (!background) {
    waitpid(left_pid, NULL, 0);
    waitpid(right_pid, NULL, 0);
  }
}

// ---------------------------- processLine(char *line) -------------------------
// Purpose:
// - Parses input line into segments based on separators (;, &, |)
// - Handles background execution and command piping
// Parameters:
// - line - input line to be processed
// ------------------------------------------------------------------------------
void processLine(char *line) {
  char *tokens[MAX_TOKENS];
  int token_count = 0;

  // Tokenize the input line
  char *token = strtok(line, " \t");
  while (token != NULL && token_count < MAX_TOKENS - 1) {
    tokens[token_count++] = token;
    token = strtok(NULL, " \t");
  }
  tokens[token_count] = NULL;

  // Segment structure to hold command segments
  struct {
    char **tokens;
    bool background;
  } segments[MAX_SEGMENTS];
  int num_segments = 0;

  int i = 0;
  while (i < token_count) {
    int start = i;

    // Find the end of the current segment
    while (i < token_count && !equal(tokens[i], ";") && !equal(tokens[i], "&")) {
      i++;
    }

    bool background = false;
    int end = i;

    // Check if a separator was found
    if (i < token_count) {
      background = equal(tokens[i], "&");
      i++;
    }

    // Extract tokens for this segment
    int seg_len = end - start;
    char **seg_tokens = malloc((seg_len + 1) * sizeof(char *));
    for (int j = 0; j < seg_len; j++) {
      seg_tokens[j] = tokens[start + j];
    }
    seg_tokens[seg_len] = NULL;

     // Store the segment
    segments[num_segments].tokens = seg_tokens;
    segments[num_segments].background = background;
    num_segments++;
  }

  // Execute each segment
  for (int s = 0; s < num_segments; s++) {
    char **tokens = segments[s].tokens;
    bool background = segments[s].background;

    int pipe_pos = -1;
    for (int i = 0; tokens[i] != NULL; i++) {
      if (equal(tokens[i], "|")) {
        pipe_pos = i;
        break;
      }
    }
    // If a pipe is found, split the command into left and right segments
    if (pipe_pos != -1) {
      char **left = tokens;
      left[pipe_pos] = NULL;
      char **right = &tokens[pipe_pos + 1];
      execute_pipeline(left, right, background);
    } else {
      execute_single_command(tokens, background);
    }
    free(tokens);
  }
}

// -------------------------- int interactiveShell() ----------------------------
// Purpose:
// - Runs the interactive shell loop
// - Handles user input, command history, and command execution
// ------------------------------------------------------------------------------
int interactiveShell() {
  bool should_run = true;
  char *line = calloc(1, MAXLINE);
  static char *last_command = NULL;
 
  // Prompt for user input
  while (should_run) { 
    printf(PROMPT);
    fflush(stdout);
    int n = fetchline(&line);

    // Check for exit command
    if (n == -1 || equal(line, "exit")) {
      should_run = false;
      continue;
    }

    // Handle empty line
    if (equal(line, "")) continue;

    // Handle command history
    if (equal(line, "!!")) {
      // Check if there is a last command
      if (!last_command) {
        printf("No commands in history.\n");
        continue;
      }
      strncpy(line, last_command, MAXLINE);
      line[MAXLINE - 1] = '\0';
      printf("osh> %s\n", line);
    } 
    else {
      free(last_command);
      last_command = strdup(line);
    }

    processLine(line);
  }
  // Free allocated memory
  free(line);
  free(last_command);
  return 0;
}

// ---------------------------- int runTests() ---------------------------------
// Purpose:
// - Runs built-in test cases for the shell
// - Tests various command scenarios
// ------------------------------------------------------------------------------
int runTests() {
  printf("*** Running basic tests ***\n");

  // Test cases for the shell
  char lines[7][MAXLINE] = {"ls", "ls -al", "ls & whoami ;", "ls > junk.txt", "cat < junk.txt", "ls | wc", "ascii"};
  for (int i = 0; i < 7; i++) {
    printf("* %d. Testing %s *\n", i + 1, lines[i]);
    processLine(lines[i]);
  }
  return 0;
}

// ------------------------ bool equal(char *a, char *b) ------------------------
// Purpose:
// - Compares two strings for equality
// Parameters:
// - a - first string
// - b - second string
// ------------------------------------------------------------------------------
bool equal(char *a, char *b) { return strcmp(a, b) == 0; }

// -------------------------- int fetchline(char **line) ------------------------
// Purpose:
// - Reads a line from stdin
// Parameters:
// - line - pointer to the line buffer
// ------------------------------------------------------------------------------
int fetchline(char **line) {
  size_t len = 0;
  ssize_t n = getline(line, &len, stdin);
  if (n > 0) {
    (*line)[n - 1] = '\0';
  }
  return n;
}