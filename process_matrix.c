#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "process_matrix.h"

int *sum_subcols(int a [][88], int start, int end) {
    int *totals = malloc(88 * sizeof(int));
    for (int j = 0; j < 88; j++) {
        totals[j] = 0;
        for (int i = start; i < end; i++) {
            totals[j] += a[i][j];
        }
    }
    return totals;
}

int *sum_with_period12(int a[88]) {
    int *totals = malloc(12 * sizeof(int));
    for (int k = 0; k < 12; k++) {
        for(int j = k; j < 88; j += 12) {
            totals[k] += a[j];
        }
    }
    return totals;
}

int *centroids_period12(int a[88]) {
    double centroids[12]; 
    for (int k = 0; k < 12; k++) {
        double num = 0, den = 0;
        for(int j = k; j < 88; j += 12) {
            den += a[j];
            num += a[j] * j;
        }
        centroids[k] = num / den;
    }
    // Quantize the centroids:
    int *centroids_int = malloc(12 * sizeof(int));
    for (int k = 0; k < 12; k++) {
        centroids[k] = round((centroids[k] - k) / 12.0) * 12 + k;
        centroids_int[k] = (int) centroids[k];
    }
    return centroids_int;
}

