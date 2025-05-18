// ----------------------------------- shell.h -----------------------------------
// Author: Dhruva Pyapali and Veer Saini
// Course: CSS 430
// Creation Date: Apr 10, 2025
// Last Modified: Apr 13, 2025
// ---------------------------------------------------------------------------------
// Purpose:
// This header file defines the interface for a UNIX-like shell implementation.
// It provides constants, data structures, and function prototypes to support:
// - Interactive command execution with background processes (&)
// - Input/output redirection (< and >)
// - Command piping (|)
// - Command history (!!)
// - Built-in test cases
// - ASCII art
// - Error handling for invalid commands
// ---------------------------------------------------------------------------------

#ifndef SHELL_H
#define SHELL_H

#include <assert.h>  // assert
#include <fcntl.h>   // O_RDWR, O_CREAT
#include <stdbool.h> // bool
#include <stdio.h>   // printf, getline
#include <stdlib.h>  // calloc
#include <string.h>  // strcmp
#include <unistd.h>  // execvp
#include <sys/wait.h>

// Config constants
#define MAXLINE 80  // max length of input line
#define PROMPT "osh> " // shell prompt
#define MAX_TOKENS 40 // max number of tokens
#define MAX_SEGMENTS 10 // max number of segments

// Pipe constants
#define RD 0 // read end of pipe
#define WR 1 // write end of pipe

// Functions
bool equal(char *a, char *b); // compare two strings
int fetchline(char **line); // read a line from stdin
int interactiveShell(); // run interactive shell
int runTests(); // run test cases
void processLine(char *line); // process a line of input
int main(int argc, char **argv); // main function
void print_ascii_art(); // print ASCII art

#endif