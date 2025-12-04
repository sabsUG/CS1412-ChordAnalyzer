#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "process_matrix.h"

int *sum_subcols(int *a, int start, int end) {
    int *totals = malloc(88 * sizeof(int));

    for (int j = 0; j < 88; j++) {
        totals[j] = 0;

        for (int i = start; i < end; i++) {
            totals[j] += a[i * 88 + j];
        }
    }

    return totals;
}


int *sum_with_period12(int a[88]) {
    int *totals = malloc(12 * sizeof(int));

    for (int k = 0; k < 12; k++)
        totals[k] = 0;   // IMPORTANT FIX

    for (int k = 0; k < 12; k++) {
        for(int j = k; j < 88; j += 12) {
            totals[k] += a[j];
        }
    }
    return totals;
}


int *centroids_period12(int vols[88]) {
    int *centroids = malloc(12 * sizeof(int));

    for (int k = 0; k < 12; k++) {
        int sum = 0;
        int count = 0;

        for (int j = k; j < 88; j += 12) {
            sum += vols[j];
            count++;
        }

        centroids[k] = (count > 0) ? (sum / count) : 0;
    }

    return centroids;
}


