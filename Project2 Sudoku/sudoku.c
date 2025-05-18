// Sudoku puzzle verifier and solver

#include <assert.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// Global variables for threaded validation
bool *regionResults;
int psize_global;

// Structure for passing data to threads
typedef struct {
    int row;
    int column;
    int **grid;
} parameters;

// Check a single row
void *checkRow(void *params) {
    parameters *p = (parameters *)params;
    int size = psize_global;
    bool seen[size+1];
    for (int i = 1; i <= size; i++) seen[i] = false;
    int r = p->row;
    for (int c = 1; c <= size; c++) {
        int val = p->grid[r][c];
        if (val < 1 || val > size || seen[val]) {
            regionResults[r-1] = false;
            free(p);
            return NULL;
        }
        seen[val] = true;
    }
    regionResults[r-1] = true;
    free(p);
    return NULL;
}

// Check a single column
void *checkColumn(void *params) {
    parameters *p = (parameters *)params;
    int size = psize_global;
    bool seen[size+1];
    for (int i = 1; i <= size; i++) seen[i] = false;
    int c = p->column;
    for (int r = 1; r <= size; r++) {
        int val = p->grid[r][c];
        if (val < 1 || val > size || seen[val]) {
            regionResults[size + c - 1] = false;
            free(p);
            return NULL;
        }
        seen[val] = true;
    }
    regionResults[size + c - 1] = true;
    free(p);
    return NULL;
}

// Check a single subgrid
void *checkSubgrid(void *params) {
    parameters *p = (parameters *)params;
    int size = psize_global;
    int boxSize = (int) sqrt(size);
    bool seen[size+1];
    for (int i = 1; i <= size; i++) seen[i] = false;
    int startRow = p->row;
    int startCol = p->column;
    int boxRow = (startRow-1)/boxSize;
    int boxCol = (startCol-1)/boxSize;
    int idx = 2*size + boxRow*boxSize + boxCol;
    for (int r = startRow; r < startRow + boxSize; r++) {
        for (int c = startCol; c < startCol + boxSize; c++) {
            int val = p->grid[r][c];
            if (val < 1 || val > size || seen[val]) {
                regionResults[idx] = false;
                free(p);
                return NULL;
            }
            seen[val] = true;
        }
    }
    regionResults[idx] = true;
    free(p);
    return NULL;
}

// takes puzzle size and grid[][] representing sudoku puzzle
// and tow booleans to be assigned: complete and valid.
// row-0 and column-0 is ignored for convenience, so a 9x9 puzzle
// has grid[1][1] as the top-left element and grid[9]9] as bottom right
// A puzzle is complete if it can be completed with no 0s in it
// If complete, a puzzle is valid if all rows/columns/boxes have numbers from 1
// to psize For incomplete puzzles, we cannot say anything about validity
void checkPuzzle(int psize, int **grid, bool *complete, bool *valid) {
    psize_global = psize;

    // 1) Check completeness (no zeros)
    *complete = true;
    for (int r = 1; r <= psize; r++) {
        for (int c = 1; c <= psize; c++) {
            if (grid[r][c] == 0) {
                *complete = false;
                break;
            }
        }
        if (!*complete) break;
    }
    // If incomplete, we cannot validate
    if (!*complete) {
        *valid = false;
        return;
    }

    // 2) Spawn threads to check rows, columns, and boxes
    int totalRegions = psize * 3;
    regionResults = malloc(totalRegions * sizeof(bool));
    pthread_t threads[totalRegions];
    int tidx = 0;

    // Rows
    for (int i = 1; i <= psize; i++) {
        parameters *data = malloc(sizeof(parameters));
        data->row = i;
        data->column = 0;
        data->grid = grid;
        pthread_create(&threads[tidx], NULL, checkRow, data);
        tidx++;
    }
    // Columns
    for (int i = 1; i <= psize; i++) {
        parameters *data = malloc(sizeof(parameters));
        data->row = 0;
        data->column = i;
        data->grid = grid;
        pthread_create(&threads[tidx], NULL, checkColumn, data);
        tidx++;
    }
    // Boxes
    int boxSize = (int) sqrt(psize);
    for (int br = 0; br < boxSize; br++) {
        for (int bc = 0; bc < boxSize; bc++) {
            parameters *data = malloc(sizeof(parameters));
            data->row = br * boxSize + 1;
            data->column = bc * boxSize + 1;
            data->grid = grid;
            pthread_create(&threads[tidx], NULL, checkSubgrid, data);
            tidx++;
        }
    }

    // 3) Join threads and combine results
    *valid = true;
    for (int i = 0; i < tidx; i++) {
        pthread_join(threads[i], NULL);
        if (!regionResults[i]) *valid = false;
    }
    free(regionResults);
}

// takes filename and pointer to grid[][]
// returns size of Sudoku puzzle and fills grid
int readSudokuPuzzle(char *filename, int ***grid) {
  FILE *fp = fopen(filename, "r");
  if (fp == NULL) {
    printf("Could not open file %s\n", filename);
    exit(EXIT_FAILURE);
  }
  int psize;
  fscanf(fp, "%d", &psize);
  int **agrid = (int **)malloc((psize + 1) * sizeof(int *));
  for (int row = 1; row <= psize; row++) {
    agrid[row] = (int *)malloc((psize + 1) * sizeof(int));
    for (int col = 1; col <= psize; col++) {
      fscanf(fp, "%d", &agrid[row][col]);
    }
  }
  fclose(fp);
  *grid = agrid;
  return psize;
}

// takes puzzle size and grid[][]
// prints the puzzle
void printSudokuPuzzle(int psize, int **grid) {
  printf("%d\n", psize);
  for (int row = 1; row <= psize; row++) {
    for (int col = 1; col <= psize; col++) {
      printf("%d ", grid[row][col]);
    }
    printf("\n");
  }
  printf("\n");
}

// takes puzzle size and grid[][]
// frees the memory allocated
void deleteSudokuPuzzle(int psize, int **grid) {
  for (int row = 1; row <= psize; row++) {
    free(grid[row]);
  }
  free(grid);
}

// expects file name of the puzzle as argument in command line
int main(int argc, char **argv) {
  if (argc != 2) {
    printf("usage: ./sudoku puzzle.txt\n");
    return EXIT_FAILURE;
  }
  // grid is a 2D array
  int **grid = NULL;
  // find grid size and fill grid
  int sudokuSize = readSudokuPuzzle(argv[1], &grid);
  bool valid = false;
  bool complete = false;
  checkPuzzle(sudokuSize, grid, &complete, &valid);
  printf("Complete puzzle? ");
  printf(complete ? "true\n" : "false\n");
  if (complete) {
    printf("Valid puzzle? ");
    printf(valid ? "true\n" : "false\n");
  }
  printSudokuPuzzle(sudokuSize, grid);
  deleteSudokuPuzzle(sudokuSize, grid);
  return EXIT_SUCCESS;
}
