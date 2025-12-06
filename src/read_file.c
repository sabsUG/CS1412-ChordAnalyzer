// CS1412   –  Programming Principles  
// Project:    Chord Analyzer (polynizer)  
// Authors:    Josue Rodriguez, Arianna Saborío  
// Instructor: Dr. Arturo Camacho

#include "read_file.h"
#include <stdio.h>
#include <stdlib.h>

#define DOUBLE_SIZE 30

int *read_vols(char *filename, int *n_rows) {
    FILE *fp = fopen(filename, "rb");
    if (!fp) return NULL;

    // Move to end of file to determine size in bytes
    fseek(fp, 0, SEEK_END);
    long total_bytes = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    // Each row contains exactly 88 bytes
    *n_rows = total_bytes / 88;

    // Allocate array for ALL rows * 88 columns
    int total_ints = (*n_rows) * 88;
    int *a = malloc(total_ints * sizeof(int));
    if (!a) return NULL;

    // Read entire matrix byte-by-byte
    for (int i = 0; i < total_ints; i++) {
        int c = getc(fp);
        if (c == EOF) c = 0; // safety — shouldn't happen
        a[i] = c;           // store raw byte into int
    }

    fclose(fp);
    return a;
}


double *read_times(char *filename, int *n_lines) {
    FILE *fp;
    fp = fopen(filename, "r");
    int c;
    char str[DOUBLE_SIZE];

    *n_lines = 0;
    int last = '\n';
    while ((c = getc(fp)) != EOF) {
        if (c == '\n')
            (*n_lines)++;
        last = c;
    }
    if (last != '\n')  // count last line
        (*n_lines)++;

    double *times = malloc(*n_lines * sizeof(double));

    fseek(fp, 0, SEEK_SET);
    for (int i = 0; i < *n_lines; i++) {
        fgets(str, DOUBLE_SIZE, fp);
        times[i] = strtod(str, NULL);
    }

    fclose(fp);
    return times;
}
