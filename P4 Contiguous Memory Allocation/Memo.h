#ifndef MEMO_H
#define MEMO_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MEMSIZE   80
#define FREE      '.'
#define LINESIZE 128

// Describes a free block by [s, e)
typedef struct {
    int s;
    int e;
} PAIR;

// Allocation strategies
PAIR* doAllocFirst(char* mem, int size);
PAIR* doAllocBest (char* mem, int size);
PAIR* doAllocWorst(char* mem, int size);

// Operations on the pool
void stomp     (char* mem, char name, int start, int size);
void doAlloc   (char* mem, char name, int size, char algo);
void doShow    (char* mem);
void doCompact (char* mem);
void doRead    (char* mem, char* filename);
void doCommand (char* mem, char* cmd);
void help      (void);

#endif // MEMO_H