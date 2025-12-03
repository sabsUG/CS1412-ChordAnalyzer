#include "read_file.h"
#include <stdlib.h>

void read_file(int n, int a[n][88]) {
    for (int i = 0; i < n; i++) { // For each row:
        for(int j = 0; j < 88; j++){
            a[i][j] = rand() % 256;
        }
    }
}
