#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "read_file.h"
#include "process_matrix.h"
#include "dictADT.h"

/* Convert duodecimal character to integer (0..11). */
static int duodec_to_int(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c == 'A') return 10;
    if (c == 'B') return 11;
    return 0;
}

/* Convert integer 0..11 to duodecimal character. */
static char int_to_duodec(int n) {
    static const char map[12] = "0123456789AB";
    return map[n % 12];
}

/* Names of pitch classes from A (0) up to Ab (11). */
static const char *pc_names[12] = {
    "A","Bb","B","C","Db","D","Eb","E","F","Gb","G","Ab"
};

/* Struct for pitch class and its volume. */
typedef struct {
    int pc;
    int vol;
} PCVol;

/* Struct for one dictionary meaning after processing. */
typedef struct {
    int bass_pc;      // absolute bass class (0..11)
    int root_pc;      // absolute root class (0..11)
    int char_note;    // quantized centroid (from centroids array)
    int delta;        // |char_note - main_centroid|
    const char *root_name;
    char type[64];    // chord suffix, may be empty
} BRT;

/* Sort BRT array by delta, then char_note, then root. */
static void sort_brt(BRT *arr, int n) {
    for (int i = 1; i < n; i++) {
        BRT key = arr[i];
        int j = i - 1;
        while (j >= 0 &&
              (arr[j].delta > key.delta ||
               (arr[j].delta == key.delta && arr[j].char_note > key.char_note) ||
               (arr[j].delta == key.delta && arr[j].char_note == key.char_note &&
                arr[j].root_pc > key.root_pc))) {
            arr[j + 1] = arr[j];
            j--;
        }
        arr[j + 1] = key;
    }
}

int main(int argc, char *argv[]) {
    if (argc != 5) {
        fprintf(stderr,"Usage: %s <vols.dat> <begin.txt> <end.txt> <dict.txt>\n", argv[0]);
        return 1;
    }

    /* Read the volume matrix (flattened) */
    int vol_size;
    int *vols = read_vols(argv[1], &vol_size);

    /* Read start/end times; n_chords is determined by read_times() */
    int n_chords;
    double *start_times = read_times(argv[2], &n_chords);
    double *end_times   = read_times(argv[3], &n_chords);

    /* Create dictionary for chord words */
    Dict dict = create_dict(argv[4]);

    /* Process each chord interval */
    for (int idx = 0; idx < n_chords; idx++) {
        /* Convert time to row index */
        int start_row = (int)(start_times[idx] * 11025 / 256);
        int end_row   = (int)(end_times[idx]   * 11025 / 256);
        if (start_row < 0) start_row = 0;

        /* Step 1: Sum volumes per note */
        int *totals = sum_subcols(vols, start_row, end_row);

        /* Step 2: Sum totals into 12 pitch classes and drop < 3.5% */
        int *pc_totals = sum_with_period12(totals);
        int total_sum = 0;
        for (int i = 0; i < 12; i++) total_sum += pc_totals[i];
        for (int i = 0; i < 12; i++) {
            if (pc_totals[i] < (int)(0.035 * total_sum)) pc_totals[i] = 0;
        }

        /* Step 3: Quantized centroids for each pitch class */
        int *centroids = centroids_period12(totals);

        /* Step 4: Sort pitch classes by volume */
        PCVol pcvol[12];
        for (int i = 0; i < 12; i++) {
            pcvol[i].pc  = i;
            pcvol[i].vol = pc_totals[i];
        }
        /* simple bubble sort (descending) */
        for (int i = 0; i < 12 - 1; i++) {
            for (int j = i + 1; j < 12; j++) {
                if (pcvol[j].vol > pcvol[i].vol) {
                    PCVol tmp = pcvol[i];
                    pcvol[i] = pcvol[j];
                    pcvol[j] = tmp;
                }
            }
        }

        /* Step 5: Form the 3-character duodecimal word */
        char word[4]; word[3] = '\0';
        for (int k = 1; k <= 3; k++) {
            if (pcvol[k].vol > 0) {
                int interval = (pcvol[k].pc - pcvol[0].pc + 12) % 12;
                word[k-1] = int_to_duodec(interval);
            } else {
                word[k-1] = '0';
            }
        }

        /* Step 6: Dictionary lookup; search() returns meanings or NULL */
        char *meaning_line = search(dict, word);

        /* Prepare up to 3 chord names */
        char chord_names[3][64];
        int chord_count_out = 0;

        if (!meaning_line || *meaning_line == '\0') {
            /* If no match, output just the main class name */
            snprintf(chord_names[0], sizeof(chord_names[0]), "%s",
                     pc_names[pcvol[0].pc]);
            chord_count_out = 1;
        } else {
            /* Count how many meanings separated by ';' */
            int mcount = 1;
            for (char *p = meaning_line; *p; p++)
                if (*p == ';') mcount++;

            BRT *meanings = malloc(mcount * sizeof(BRT));
            char *copy    = strdup(meaning_line);
            char *token   = strtok(copy, ";");
            int m = 0;

            /* Step 7: Parse each meaning */
            while (token && m < mcount) {
                /* Format: bass,root,type */
                char *c1 = strchr(token, ',');
                char *c2 = strchr(c1 + 1, ',');

                char bass_char = token[0];

                /* Skip spaces to find root char */
                char *p = c1 + 1;
                while (*p == ' ') p++;
                char root_char = *p;

                /* Capture everything after second comma as type (trim leading spaces) */
                char *q = c2 + 1;
                while (*q == ' ') q++;
                int tlen = 0;
                while (q[tlen] && q[tlen] != ';' && q[tlen] != '\n' && q[tlen] != '\r')
                    tlen++;
                strncpy(meanings[m].type, q, tlen);
                meanings[m].type[tlen] = '\0';

                /* Convert relative classes to absolute */
                int bass_rel = duodec_to_int(bass_char);
                int root_rel = duodec_to_int(root_char);

                int bass_abs = (bass_rel + pcvol[0].pc) % 12;
                int root_abs = (root_rel + pcvol[0].pc) % 12;

                meanings[m].bass_pc   = bass_abs;
                meanings[m].root_pc   = root_abs;
                meanings[m].char_note = centroids[bass_abs];
                meanings[m].delta     = abs(centroids[bass_abs] - centroids[pcvol[0].pc]);
                meanings[m].root_name = pc_names[root_abs];

                token = strtok(NULL, ";");
                m++;
            }

            /* Step 8: Sort by delta (distance from main centroid), then char_note, then root_pc */
            sort_brt(meanings, mcount);

            /* Step 9: Construct chord names and remove duplicates. */
            int ucount = 0;
            for (int i = 0; i < mcount && ucount < 3; i++) {
                char candidate[64];
                if (meanings[i].type[0] == '\0') {
                    snprintf(candidate, sizeof(candidate), "%s",
                             meanings[i].root_name);
                } else {
                    snprintf(candidate, sizeof(candidate), "%s%s",
                             meanings[i].root_name, meanings[i].type);
                }

                /* Check for duplicates */
                int duplicate = 0;
                for (int j = 0; j < ucount; j++) {
                    if (strcmp(candidate, chord_names[j]) == 0) {
                        duplicate = 1;
                        break;
                    }
                }
                if (!duplicate) {
                    snprintf(chord_names[ucount], sizeof(chord_names[ucount]),
                             "%s", candidate);
                    ucount++;
                }
            }
            chord_count_out = ucount;

            free(copy);
            free(meanings);
        }

        /* Print chord index, time, and up to three suggestions. */
        printf("%d: %.2f\n", idx + 1, start_times[idx]);
        for (int i = 0; i < chord_count_out; i++) {
            printf("%s\n", chord_names[i]);
        }
        printf("\n");

        /* Free resources for this chord. */
        free(totals);
        free(pc_totals);
        free(centroids);
    }

    /* Clean up global structures. */
    free(vols);
    free(start_times);
    free(end_times);

    return 0;
}
