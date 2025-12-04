#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "read_file.h"
#include "process_matrix.h"
#include "dictADT.h"

/* Helpers for Duodecimal conversion */
static int duodec_to_int(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c == 'A') return 10;
    if (c == 'B') return 11;
    return 0;
}

static char int_to_duodec(int n) {
    static const char map[12] = "0123456789AB";
    return map[n % 12];
}

static const char *pc_names[12] = {
    "A","Bb","B","C","Db","D","Eb","E","F","Gb","G","Ab"
};

typedef struct {
    int pc;
    int vol;
} PCVol;

typedef struct {
    int bass_pc;
    int root_pc;
    int char_note;
    const char *root_name;
    char type[64];
} BRT;

/* Step 8: Sort by characteristic note (low to high). Stable sort.  */
void stable_sort_meanings_BRT(BRT *meanings, int m) {
    for (int i = 1; i < m; i++) {
        BRT temp = meanings[i];
        int j = i - 1;

        while (j >= 0 && meanings[j].char_note > temp.char_note) {
            meanings[j + 1] = meanings[j];
            j--;
        }

        meanings[j + 1] = temp;
    }
}



int main(int argc, char *argv[]) {
    if (argc != 5) {
        fprintf(stderr,"Usage: %s <vols.dat> <begin.txt> <end.txt> <dict.txt>\n", argv[0]);
        return 1;
    }

    /* Print Headers */
    printf("%s\n%s\n%s\n\n", argv[1], argv[2], argv[3]);

    int vol_size;
    int *vols = read_vols(argv[1], &vol_size);

    int n_chords;
    double *start_times = read_times(argv[2], &n_chords);
    double *end_times   = read_times(argv[3], &n_chords);

    Dict dict = create_dict(argv[4]);

    for (int idx = 0; idx < n_chords; idx++) {
        /* Convert Time to Row. Rate: 11025/256 Hz [cite: 486] */
        int start_row = (int)(start_times[idx] * 11025.0 / 256.0);
        int end_row   = (int)(end_times[idx]   * 11025.0 / 256.0);

        /* Step 1: Sum volumes */
        int *totals = sum_subcols(vols, start_row, end_row);

        /* Step 2: Pitch Class Totals and Thresholding [cite: 503] */
        int *pc_totals = sum_with_period12(totals);
        long total_sum = 0;
        for (int i = 0; i < 12; i++) total_sum += pc_totals[i];
        
        /* Use integer arithmetic for strict 3.5% check */
        for (int i = 0; i < 12; i++) {
            if ((long)pc_totals[i] * 1000 < total_sum * 35) {
                pc_totals[i] = 0;
            }
        }

        /* Step 3: Quantized Centroids */
        int *centroids = centroids_period12(totals);

        /* Step 4: Sort classes by volume (descending) [cite: 507] */
        PCVol pcvol[12];
        for (int i = 0; i < 12; i++) {
            pcvol[i].pc  = i;
            pcvol[i].vol = pc_totals[i];
        }
        /* Bubble sort */
        for (int i = 0; i < 12 - 1; i++) {
            for (int j = i + 1; j < 12; j++) {
                if (pcvol[j].vol > pcvol[i].vol) {
                    PCVol tmp = pcvol[i];
                    pcvol[i] = pcvol[j];
                    pcvol[j] = tmp;
                }
            }
        }

        /* Step 5: Generate Word relative to loudest [cite: 508] */
        char word[13]; 
        int w_idx = 0;
        /* Only include classes that survived the threshold (vol > 0) */
        for (int k = 1; k < 12; k++) {
            if (pcvol[k].vol > 0) {
                int interval = (pcvol[k].pc - pcvol[0].pc + 12) % 12;
                word[w_idx++] = int_to_duodec(interval);
            }
        }
        word[w_idx] = '\0';

        /* Step 6: Dictionary Search [cite: 510] 
           Since you are using trie.c, 'search' maps to trie_search_longest_prefix,
           so we do NOT need a loop here. */
        char *meaning_line = search(dict, word);

        char chord_names[3][64];
        int chord_count_out = 0;

        if (!meaning_line || *meaning_line == '\0') {
            /* No match found */
            snprintf(chord_names[0], sizeof(chord_names[0]), "%s", pc_names[pcvol[0].pc]);
            chord_count_out = 1;
        } else {
            /* Parse meanings separated by ';' */
            int mcount = 0;
            for (char *p = meaning_line; *p; p++) if (*p == ';') mcount++;

            BRT *meanings = malloc(mcount * sizeof(BRT));
            char *copy    = strdup(meaning_line);
            char *token   = strtok(copy, ";");
            int m = 0;

            /* Step 7: Interpret Meanings [cite: 512] */
            while (token && m < mcount) {

                // Trim leading spaces
                while (*token == ' ' || *token == '\n' || *token == '\t')
                    token++;

                char *c1 = strchr(token, ',');
                char *c2 = (c1 ? strchr(c1 + 1, ',') : NULL);

                if (c1 && c2) {
                    // bass char = first non-space character
                    char bass_char = token[0];

                    // root char = first non-space character after c1
                    char *root_ptr = c1 + 1;
                    while (*root_ptr == ' ') root_ptr++;
                    char root_char = *root_ptr;

                    // type = after second comma
                    char *type_start = c2 + 1;
                    while (*type_start == ' ') type_start++;

                    strncpy(meanings[m].type, type_start, 63);
                    meanings[m].type[63] = '\0';

                    // trim trailing whitespace
                    size_t t = strlen(meanings[m].type);
                    while (t > 0 && meanings[m].type[t-1] <= ' ')
                        meanings[m].type[--t] = '\0';

                    // convert bass/root
                    int bass_rel = duodec_to_int(bass_char);
                    int root_rel = duodec_to_int(root_char);

                    int bass_abs = (bass_rel + pcvol[0].pc) % 12;
                    int root_abs = (root_rel + pcvol[0].pc) % 12;

                    meanings[m].char_note = centroids[bass_abs];
                    meanings[m].root_name = pc_names[root_abs];

                    m++;
                }

                token = strtok(NULL, ";");
            }


            /* Step 8: Sort  */
            stable_sort_meanings_BRT(meanings, m);

            /* Step 9 & 10: Deduplicate and select top 3 [cite: 537] */
            int ucount = 0;
            for (int i = 0; i < m && ucount < 3; i++) {
                char candidate[64];
                if (meanings[i].type[0] == '\0') {
                    snprintf(candidate, sizeof(candidate), "%s", meanings[i].root_name);
                } else {
                    snprintf(candidate, sizeof(candidate), "%s%s", meanings[i].root_name, meanings[i].type);
                }

                int duplicate = 0;
                for (int j = 0; j < ucount; j++) {
                    if (strcmp(candidate, chord_names[j]) == 0) {
                        duplicate = 1;
                        break;
                    }
                }
                if (!duplicate) {
                    snprintf(chord_names[ucount], sizeof(chord_names[ucount]), "%s", candidate);
                    ucount++;
                }
            }
            chord_count_out = ucount;

            free(copy);
            free(meanings);
        }

        /* Output */
        printf("%d: %.2f\n", idx + 1, start_times[idx]);
        for (int i = 0; i < chord_count_out; i++) {
            printf("%s\n", chord_names[i]);
        }
        printf("\n");


        if (idx == 4) {  // chord #5 (0-based index)
        printf("\nDEBUG CHORD 5:\n");
        printf("Rows: %d to %d\n", start_row, end_row);

        printf("PC totals BEFORE threshold:\n");
        for (int i = 0; i < 12; i++) printf("%d ", pc_totals[i]);
        printf("\nTotal sum = %ld\n", total_sum);

        // recompute thresholded:
        printf("PC totals AFTER threshold:\n");
        for (int i = 0; i < 12; i++) printf("%d ", pc_totals[i]);
        printf("\n");

        printf("Sorted PC order (pc,vol):\n");
        for (int i = 0; i < 12; i++)
            printf("(%d,%d) ", pcvol[i].pc, pcvol[i].vol);
        printf("\n");

        printf("WORD: %s\n", word);
        }


        free(totals);
        free(pc_totals);
        free(centroids);
    }

    free(vols);
    free(start_times);
    free(end_times);

    return 0;
}