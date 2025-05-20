// ============================================================================
// Memo.c : simulation of a memory allocator
// ============================================================================

#include "Memo.h"

// ============================================================================
// Allocate via F/B/W
// ============================================================================
void doAlloc(char* mem, char name, int size, char algo) {
  PAIR* p = NULL;
  if      (algo == 'F') { 
    p = doAllocFirst(mem, size);
  } else if (algo == 'B') {
    p = doAllocBest (mem, size);
  } else if (algo == 'W') {
    p = doAllocWorst(mem, size);
  }

  if (p) {
    stomp(mem, name, p->s, size);
    free(p);
  } else {
    printf("Cannot find %d free bytes\n", size);
  }
}

// ============================================================================
// First‐Fit
// ============================================================================
PAIR* doAllocFirst(char* mem, int size) {
  for (int i = 0; i + size <= MEMSIZE; ++i) {
    if (mem[i] != FREE) {
      continue;
    }
    // Find the end of the free block
    int j = i;
    while (j < MEMSIZE && mem[j] == FREE) {
      ++j;
    }
    // Check if the block is large enough
    if (j - i >= size) {
      PAIR* p = malloc(sizeof(PAIR));
      p->s = i; p->e = j;
      return p;
    }
    i = j;
  }
  // No suitable block found  
  return NULL; 
}

// ============================================================================
// Best‐Fit
// ============================================================================
PAIR* doAllocBest(char* mem, int size) {
    int bestSize = MEMSIZE + 1, bs = -1, be = -1;
    for (int i = 0; i < MEMSIZE; ++i) {
        if (mem[i] != FREE) continue;
        int j = i; while (j < MEMSIZE && mem[j] == FREE) ++j;
        int block = j - i;
        if (block >= size && block < bestSize) {
          bestSize = block;
          bs = i; be = j;
        }
        i = j;
    }
    if (bs >= 0) {
      PAIR* p = malloc(sizeof(PAIR));
      p->s = bs; p->e = be;
      return p;
    }
    // No suitable block found
    return NULL;
}

// ============================================================================
// Worst‐Fit
// ============================================================================
PAIR* doAllocWorst(char* mem, int size) {
  int worstSize = -1, ws = -1, we = -1;
  for (int i = 0; i < MEMSIZE; ++i) {
      if (mem[i] != FREE) {
        continue;
      }
      // Find the end of the free block
      int j = i; 
      while (j < MEMSIZE && mem[j] == FREE) {
        ++j;
      }
      // Check if the block is large enough
      int block = j - i;
      if (block >= size && block > worstSize) {
        worstSize = block;
        ws = i; we = j;
      }
      i = j;
  }
  if (ws >= 0) {
    PAIR* p = malloc(sizeof(PAIR));
    p->s = ws; p->e = we;
    return p;
  }
  // No suitable block found
  return NULL;
}

// ============================================================================
// Stamp 'size' bytes with 'name'
// ============================================================================
void stomp(char* mem, char name, int start, int size) {
  for (int i = start; i < start + size && i < MEMSIZE; ++i) {
    mem[i] = name;
  }
}

// ============================================================================
// Show pool
// ============================================================================
void doShow(char* mem) {
  for (int i = 0; i < MEMSIZE; ++i) putchar(mem[i]);
  putchar('\n');
}

// ============================================================================
// Compact to left
// ============================================================================
void doCompact(char* mem) {
  char buf[MEMSIZE];
  int idx = 0;
  for (int i = 0; i < MEMSIZE; ++i) {
    if (mem[i] != FREE) buf[idx++] = mem[i];
  }
  // Fill the rest with FREE
  while (idx < MEMSIZE) {
    buf[idx++] = FREE;
  }
  // Copy back to mem
  memcpy(mem, buf, MEMSIZE);
}

// ============================================================================
// Execute script
// ============================================================================
void doRead(char* mem, char* filename) {
    FILE* fp = fopen(filename, "r");
    if (!fp) {
      printf("Unable to open file: %s\n", filename);
      return;
    }
    // Read each line
    char line[LINESIZE];
    while (fgets(line, sizeof(line), fp)) {
      if (line[0]=='#' || isspace((unsigned char)line[0])) {
        continue;
      }
      // Remove comments
      doCommand(mem, line);
    } 
    // Close the file
    fclose(fp);
}

// ============================================================================
// Single command
// ============================================================================
void doCommand(char* mem, char* cmd) {
    char* tok = strtok(cmd, " \t\n");
    if (!tok) {
      return;
    }
    // Check for comments
    char op = toupper((unsigned char)tok[0]);
    if (op == 'A') {
      tok = strtok(NULL, " \t\n"); char name = toupper((unsigned char)tok[0]);
      tok = strtok(NULL, " \t\n"); int size = atoi(tok);
      tok = strtok(NULL, " \t\n"); char algo = toupper((unsigned char)tok[0]);
      doAlloc(mem, name, size, algo);

    } else if (op == 'F') {
      tok = strtok(NULL, " \t\n"); char name = toupper((unsigned char)tok[0]);
      for (int i = 0; i < MEMSIZE; ++i) {
        if (mem[i] == name) mem[i] = FREE;
      }
    } else if (op == 'S') {
      doShow(mem);

    } else if (op == 'C') {
      doCompact(mem);

    } else if (op == 'R') {
      tok = strtok(NULL, " \t\n");
      doRead(mem, tok);

    } else if (op == 'E') {
      // Exit
      exit(0);
    }
}

// ============================================================================
// Help text
// ============================================================================
void help(void) {
  printf("Commands:\n");
  printf("  A <name> <size> <F|B|W>   Allocate\n");
  printf("  F <name>                  Free\n");
  printf("  S                         Show\n");
  printf("  C                         Compact\n");
  printf("  R <filename>              Read script\n");
  printf("  E                         Exit\n");
}

// ============================================================================
// Main loop
// ============================================================================
int main(void) {
  char mem[MEMSIZE];
  memset(mem, FREE, MEMSIZE);
  help();
  while (1) {
    printf("Memo> ");
    char line[LINESIZE];
    if (!fgets(line, sizeof(line), stdin)) {
      break;
    }
    doCommand(mem, line);
  }
  return 0;
}