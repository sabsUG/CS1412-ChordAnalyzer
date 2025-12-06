#ifndef PROCESS_MATRIX_H
#define PROCESS_MATRIX_H

/* Sum rows [start, end) into one 88-element vector. */
int *sum_subcols(int *a, int start, int end);

/* Fold 88-key vector into 12 pitch classes. */
int *sum_with_period12(int a[88]);

/* Quantized centroids (characteristic notes) per pitch class. */
int *centroids_period12(int vols[88]);

#endif