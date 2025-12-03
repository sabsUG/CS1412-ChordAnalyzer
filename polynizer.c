#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "read_file.h"
#include "process_matrix.h"
#include "dictADT.h"

#define PRINT_INT(n) printf(#n " = %d\n", n)
#define PRINT_STR(n) printf(#n " = %s\n", n)

const char* class_names_flats[] = {"A", "Bb", "B", "C", "Db", "D", "Eb", "E", "F", "Gb", "G", "Ab"};

const char* class_names_sharps[] = {"A", "A#", "B", "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#"};

int duodec_to_int(int c) {
    switch (c) {
        case 'A':
            return 10;
            break;
        case 'B':
            return 11;
            break;
        default:
            return c - '0';
    }
}

struct pc_vol {
    int pc;
    int vol;
};

struct BRT {
    int bass;
    char root;
    char type[20];
};

char digit_to_duodec_char(int digit) {
    return "0123456789AB"[digit];
}

int compare_vols(const void *p, const void *q) { // To sort by volumen in DESCENDING order
    return ((struct pc_vol *) q)->vol - ((struct pc_vol *) p)->vol;
//    const struct pc_vol *p1 = p, *p2 = q;
//    if (p1->vol < p2->vol)
//        return 1;
//    else if (p1->vol > p2->vol)
//        return -1;
//    else
//        return 0;
}

int main(int argc, char *argv[]) {
    if (argc != 5) {
        printf("Wrong number of parameters\n");
        exit(EXIT_FAILURE);
    }
    int size;
    int *vols = read_vols(argv[1], &size);
    int n_chords;
    double *start_times = read_times(argv[2], &n_chords);
    for (int i = 0; i < n_chords; i++)
        printf("%f\n", start_times[i]);
    double *end_times = read_times(argv[3], &n_chords);
    for (int i = 0; i < n_chords; i++)
        printf("%f\n", end_times[i]);
    Dict d = create_dict(argv[4]);
    // Now it's time to process the chords
    for(int i=0; i<1; i++) {
        int start_row, end_row;
        start_row = start_times[i] * 11025 / 256;
        end_row = end_times[i] * 11025 / 256;
        PRINT_INT(start_row);
        PRINT_INT(end_row);
        int *totals = sum_subcols(vols, start_row, end_row);
        printf("Note volumes:\n");
        for(int i=0; i<88; i++)
            printf("%d ", totals[i]);
        printf("\n");
        // Step 2
        int *class_totals = sum_with_period12(totals);
        printf("Class totals:\n");
        int total = 0;
        for(int i=0; i<12; i++) {
            total += class_totals[i];
            printf("%d ", class_totals[i]);
        }
        printf("\n");
        printf("Total volume: %d, 3.5%%: %f\n", total, 0.035*total);
        for(int i=0; i<12; i++) {
            if (class_totals[i] < 0.035*total)
                class_totals[i] = 0;
        }
        printf("After removing values below 3.5%%:\n");
        for(int i=0; i<12; i++) {
            total += class_totals[i];
            printf("%d ", class_totals[i]);
        }
        printf("\n");
        // Step 3
        printf("Quantized centroids:\n");
        int * q_cent = centroids_period12(totals);
        for(int i=0; i<12; i++) {
            printf("%d ", q_cent[i]);
        }
        printf("\n");
        // Step 4
        struct pc_vol pc_vol[12];
        printf("Before sorting:\n");
        for (int i=0; i<12; i++) {
            pc_vol[i].pc = i;
            pc_vol[i].vol = class_totals[i];
            printf("pc: %d, vol: %d\n", pc_vol[i].pc, pc_vol[i].vol);
        }
        qsort(pc_vol, 12, sizeof(struct pc_vol), compare_vols);
        printf("After sorting:\n");
        for (int i=0; i<12; i++) {
            printf("pc: %d, vol: %d\n", pc_vol[i].pc, pc_vol[i].vol);
        }
        // Step 5 (building the word)
        char word[12] = {'\0'};
        for (int i=1; i<12; i++) {
            if (pc_vol[i].vol > 0)
                word[i-1] = digit_to_duodec_char((pc_vol[i].pc - pc_vol[0].pc + 12) % 12);
        }
        printf("word: %s\n", word);
        // Step 6 (searching the word)
        char *meaning = search(d, word);
        char *meaning_cpy = malloc(strlen(meaning) + 1);
        strcpy(meaning_cpy, meaning);
        PRINT_STR(meaning_cpy);
        // splitting the string
        int n_meanings = 0;
        char *t = strtok(meaning_cpy, ";");
        while (t != NULL) {
            n_meanings++;
            t = strtok(NULL, ";");
        }
        // Step 7
        struct BRT *brt = malloc(n_meanings * sizeof(struct BRT)); // bass-root-type
        t = strtok(meaning, ";");
        for (int i=0; i<n_meanings; i++) {
            sscanf(t, "%c,%c,%s", &brt[i].bass, &brt[i].root, brt[i].type);
            printf("bass: %c, root: %c, type: %s\n", brt[i].bass, brt[i].root, brt[i].type);
            t = strtok(NULL, ";") + 1;
            int bass = duodec_to_int(brt[i].bass);
            int root = duodec_to_int(brt[i].root);
            printf("Relative values:\n");
            PRINT_INT(bass);
            PRINT_INT(root);
            bass = (bass + pc_vol[0].pc) % 12;
            root = (root + pc_vol[0].pc) % 12;
            printf("Absolute values:\n");
            PRINT_INT(bass);
            PRINT_INT(root);
            // Replace bass with CN
            bass = q_cent[bass];
            // Replace root with CN
            printf("With characteristic note and root name:\n");
            printf("bass: %d, root: %s\n", bass, class_names_flats[root]);
        }
        
    }
}
