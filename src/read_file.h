// CS1412   –  Programming Principles  
// Project:    Chord Analyzer (polynizer)  
// Authors:    Josue Rodriguez, Arianna Saborío  
// Instructor: Dr. Arturo Camacho

#ifndef READ_FILE_H
#define READ_FILE_H

/* Read binary volumes file (rows x 88). Writes rows into *size. */
int *read_vols(char *filename, int *size);

/* Read one double per line. Writes count into *n_lines. */
double *read_times(char *filename, int *n_lines);

#endif