#include "read_file.h"
#include <stdio.h>
#include <stdlib.h>

#define DOUBLE_SIZE 30

int *read_vols(char *filename, int *size) {
    FILE *fp;
    fp = fopen(filename, "rb");
    int c;
    *size = 0;

    while ((c = getc(fp)) != EOF)
        (*size)++;

    fseek(fp, 0, SEEK_SET);

    int *a = malloc(*size * sizeof(int));

    for (int i = 0; i < *size; i++)
        a[i] = getc(fp);

    fclose(fp);
    return a;
}

double *read_times(char *filename, int *n_lines) {
    FILE *fp;
    fp = fopen(filename, "r");
    int c;
    char str[DOUBLE_SIZE];

    *n_lines = 0;
    while ((c = getc(fp)) != EOF)
        if (c == '\n')
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
