// CS1412   –  Programming Principles  
// Project:    Chord Analyzer (polynizer)  
// Authors:    Josue Rodriguez, Arianna Saborío  
// Instructor: Dr. Arturo Camacho

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "process_matrix.h"

int *sum_subcols(int *a, int start, int end) {
    int *totals = calloc(88, sizeof(int));
    if (!totals) return NULL;

    /* Flattened matrix access: row * 88 + col */
    for (int j = 0; j < 88; j++) {
        for (int i = start; i < end; i++) {
            totals[j] += a[i * 88 + j];
        }
    }
    return totals;
}

int *sum_with_period12(int a[88]) {
    int *totals = calloc(12, sizeof(int));
    if (!totals) return NULL;

    for (int k = 0; k < 12; k++) {
        for(int j = k; j < 88; j += 12) {
            totals[k] += a[j];
        }
    }
    return totals;
}

/* * Calculates the Weighted Centroid and QUANTIZES it to the nearest valid key index.
 * Reference: Step 3 
 */
int *centroids_period12(int vols[88]) {
    int *centroids = malloc(12 * sizeof(int));
    if (!centroids) return NULL;

    for (int k = 0; k < 12; k++) {
        long long sum_prod = 0;
        long long sum_vol = 0;

        for (int j = k; j < 88; j += 12) {
            sum_prod += (long long)vols[j] * j;
            sum_vol  += vols[j];
        }

        if (sum_vol > 0) {
            /* 1. Calculate exact weighted average */
            double avg = (double)sum_prod / sum_vol;
            
            /* 2. Quantize: Find integer N such that (N*12 + k) is closest to avg */
            int n = (int)round((avg - k) / 12.0);
            
            int quant_val = k + (12 * n);

            /* Clamp to valid piano range 0-87 */
            if (quant_val < 0) quant_val = k; 
            if (quant_val > 87) quant_val = (87 - k) / 12 * 12 + k;

            centroids[k] = quant_val;
        } else {
            centroids[k] = 0;
        }
    }

    return centroids;
}