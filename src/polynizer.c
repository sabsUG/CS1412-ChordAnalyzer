#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "read_file.h"
#include "process_matrix.h"
#include "dictADT.h"

/* Small helpers for base-12 (duodecimal) conversion. */
static int duodec_to_int(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c == 'A') return 10;
    if (c == 'B') return 11;
    return 0;   // shouldn't happen for valid input
}

static char int_to_duodec(int n) {
    static const char map[] = "0123456789AB";  // let compiler pick the size
    return map[n % 12];
}

/*
 * Note names (two versions). The project lets us choose sharps or flats
 * but suggests flats. The instructions also say we should pick one system
 * and stick to it, so we detect that on the first chord only.
 */
static const char *sharp_names[12] = {
    "A","A#","B","C","C#","D","D#","E","F","F#","G","G#"
};

static const char *flat_names[12] = {
    "A","Bb","B","C","Db","D","Eb","E","F","Gb","G","Ab"
};

/* Struct for storing pitch class + volume after folding to 12 classes. */
typedef struct {
    int pc;
    int vol;
} PCVol;

/*
 * Struct to store one meaning from the dictionary after we interpret it.
 * bass_pc:     absolute pitch class of bass
 * root_pc:     absolute pitch class of root
 * char_note:   characteristic note (quantized centroid)
 * root_name:   printable note name ("C", "F#", etc.)
 * type:        the chord type (like "m", "7 sus4", "")
 */
typedef struct {
    int bass_pc;
    int root_pc;
    int char_note;
    const char *root_name;
    char type[64];
} BRT;

/* We sort meanings by characteristic note using stable insertion sort. */
static void sort_meanings_by_char_note(BRT *m, int n) {
    for (int i = 1; i < n; i++) {
        BRT temp = m[i];
        int j = i - 1;
        while (j >= 0 && m[j].char_note > temp.char_note) {
            m[j + 1] = m[j];
            j--;
        }
        m[j + 1] = temp;
    }
}

int main(int argc, char *argv[]) {

    /*
     * We expect exactly four arguments:
     * 1. binary volume matrix
     * 2. start times file
     * 3. end times file
     * 4. dictionary file
     */
    if (argc != 5) {
        fprintf(stderr,
            "Usage: %s <vols.dat> <start_times.txt> <end_times.txt> <dict.txt>\n",
            argv[0]);
        return 1;
    }

    /* Print filenames exactly like the project output format shows. */
    printf("%s\n%s\n%s\n\n", argv[1], argv[2], argv[3]);

    /* Load binary volumes (rows × 88). */
    int vol_rows = 0;
    int *vols = read_vols(argv[1], &vol_rows);

    /* Load start/end times. */
    int n_start = 0, n_end = 0;
    double *start_times = read_times(argv[2], &n_start);
    double *end_times   = read_times(argv[3], &n_end);

    /*
     * Build dictionary.
     * This uses a trie internally (extra credit), so lookup is fast
     * and longest-prefix matching is easy.
     */
    Dict dict = create_dict(argv[4]);

    if (!vols || !start_times || !end_times || !dict) {
        fprintf(stderr, "Error: failed to load data or dictionary.\n");
        free(vols);
        free(start_times);
        free(end_times);
        return 1;
    }

    /* Number of chords is min(#starts, #ends). */
    int n_chords = n_start < n_end ? n_start : n_end;

    /* We will choose sharps or flats based on the first chord only. */
    const char **pc_names = NULL;

    /* Loop through all chords. */
    for (int idx = 0; idx < n_chords; idx++) {

        /* Convert time in seconds → row index (project formula). */
        int start_row = (int)(start_times[idx] * 11025.0 / 256.0);
        int end_row   = (int)(end_times[idx]   * 11025.0 / 256.0);

        /* Clamp to matrix bounds. */
        if (start_row < 0) start_row = 0;
        if (end_row < 0) end_row = 0;
        if (start_row > vol_rows) start_row = vol_rows;
        if (end_row > vol_rows) end_row = vol_rows;

        /* Sometimes end < start; just swap to avoid an empty window. */
        if (end_row < start_row) {
            int tmp = start_row;
            start_row = end_row;
            end_row = tmp;
        }

        /* If the window collapses, expand it by one row if possible. */
        if (start_row == end_row && end_row < vol_rows) {
            end_row++;
        }

        /* 
         * Step 1: Sum over selected rows → 88-element vector.
         */
        int *totals = sum_subcols(vols, start_row, end_row);
        if (!totals) {
            fprintf(stderr, "Error: sum_subcols failed\n");
            continue;
        }

        /*
         * Step 2: Fold into 12 pitch classes + apply 3.5% threshold.
         */
        int *pc_totals = sum_with_period12(totals);
        if (!pc_totals) {
            free(totals);
            fprintf(stderr, "Error: sum_with_period12 failed\n");
            continue;
        }

        int pc_totals_raw[12];
        long total_sum = 0;
        for (int i = 0; i < 12; i++) {
            pc_totals_raw[i] = pc_totals[i];
            total_sum += pc_totals[i];
        }

        /* Zero out classes with < 3.5% of the total. */
        if (total_sum > 0) {
            for (int i = 0; i < 12; i++) {
                if ((long)pc_totals[i] * 1000 < total_sum * 35) {
                    pc_totals[i] = 0;
                }
            }
        }

        /*
         * Step 3: Compute quantized centroids (one per pitch class).
         */
        int *centroids = centroids_period12(totals);
        if (!centroids) {
            free(totals);
            free(pc_totals);
            fprintf(stderr, "Error: centroids_period12 failed\n");
            continue;
        }

        /*
         * Step 4: Sort pitch classes from loudest to softest.
         */
        PCVol pcvol[12];
        for (int i = 0; i < 12; i++) {
            pcvol[i].pc  = i;
            pcvol[i].vol = pc_totals[i];
        }

        for (int i = 0; i < 11; i++) {
            for (int j = i + 1; j < 12; j++) {
                if (pcvol[j].vol > pcvol[i].vol) {
                    PCVol tmp = pcvol[i];
                    pcvol[i] = pcvol[j];
                    pcvol[j] = tmp;
                }
            }
        }

        /*
         * Decide flats vs sharps based on the first chord only.
         * The idea is that some keys naturally use flats.
         */
        if (idx == 0) {
            /* Find loudest raw pitch class from pc_totals_raw. */
            PCVol pcvol_raw[12];
            for (int i = 0; i < 12; i++) {
                pcvol_raw[i].pc  = i;
                pcvol_raw[i].vol = pc_totals_raw[i];
            }
            for (int i = 0; i < 11; i++) {
                for (int j = i + 1; j < 12; j++) {
                    if (pcvol_raw[j].vol > pcvol_raw[i].vol) {
                        PCVol tmp = pcvol_raw[i];
                        pcvol_raw[i] = pcvol_raw[j];
                        pcvol_raw[j] = tmp;
                    }
                }
            }

            int tonic_pc = pcvol_raw[0].pc;

            /* PCs that typically appear as flats. */
            int use_flats =
                (tonic_pc == 1 || tonic_pc == 3 || tonic_pc == 4 ||
                 tonic_pc == 6 || tonic_pc == 8 || tonic_pc == 10);

            pc_names = use_flats ? flat_names : sharp_names;
        }

        if (!pc_names)
            pc_names = flat_names;

        /*
         * Step 5: Build duodecimal "word".
         * We measure intervals from the loudest pitch class.
         */
        char word[13];
        int w_idx = 0;

        for (int k = 1; k < 12; k++) {
            if (pcvol[k].vol > 0) {
                int interval = (pcvol[k].pc - pcvol[0].pc + 12) % 12;
                word[w_idx++] = int_to_duodec(interval);
            }
        }
        word[w_idx] = '\0';

        /*
         * Step 6: Look up the word in the dictionary.
         * Our dictionary uses a trie (extra credit), so this returns the
         * meanings for the *longest prefix* of the word.
         */
        char *meaning_line = search(dict, word);

        char chord_names[3][64];
        int chord_count_out = 0;

        /*
         * If we don't find anything in the dictionary,
         * the safest fallback is just the root note.
         */
        if (!meaning_line || meaning_line[0] == '\0') {
            snprintf(chord_names[0], sizeof(chord_names[0]),
                     "%s", pc_names[pcvol[0].pc]);
            chord_count_out = 1;
        } else {

            /* Count how many meanings there are (they end with ';'). */
            int mcount = 0;
            for (char *p = meaning_line; *p; p++) {
                if (*p == ';') mcount++;
            }

            BRT *meanings = malloc(mcount * sizeof(BRT));
            if (!meanings) {
                snprintf(chord_names[0], sizeof(chord_names[0]),
                         "%s", pc_names[pcvol[0].pc]);
                chord_count_out = 1;
            } else {

                /* Copy because strtok modifies the string. */
                char *copy = strdup(meaning_line);
                char *token = strtok(copy, ";");
                int m = 0;

                while (token && m < mcount) {

                    /* Trim leading whitespace just in case. */
                    while (*token == ' ' || *token == '\t' || *token == '\n')
                        token++;

                    /* Format is: bass,root,type */
                    char *c1 = strchr(token, ',');
                    char *c2 = c1 ? strchr(c1 + 1, ',') : NULL;

                    if (c1 && c2) {
                        /* First char = bass (relative). */
                        char bass_char = token[0];

                        /* Next non-space after comma = root. */
                        char *root_ptr = c1 + 1;
                        while (*root_ptr == ' ') root_ptr++;
                        char root_char = *root_ptr;

                        /* Everything after second comma = type. */
                        char *type_ptr = c2 + 1;
                        while (*type_ptr == ' ') type_ptr++;

                        strncpy(meanings[m].type, type_ptr, 63);
                        meanings[m].type[63] = '\0';

                        /* Trim trailing whitespace in type. */
                        size_t t = strlen(meanings[m].type);
                        while (t > 0 &&
                               (meanings[m].type[t - 1] == ' ' ||
                                meanings[m].type[t - 1] == '\n' ||
                                meanings[m].type[t - 1] == '\r' ||
                                meanings[m].type[t - 1] == '\t')) {
                            meanings[m].type[--t] = '\0';
                        }

                        /* Convert from relative to absolute pitch classes. */
                        int bass_rel = duodec_to_int(bass_char);
                        int root_rel = duodec_to_int(root_char);

                        int bass_abs = (bass_rel + pcvol[0].pc) % 12;
                        int root_abs = (root_rel + pcvol[0].pc) % 12;

                        meanings[m].bass_pc   = bass_abs;
                        meanings[m].root_pc   = root_abs;
                        meanings[m].root_name = pc_names[root_abs];
                        meanings[m].char_note = centroids[bass_abs];

                        m++;
                    }

                    token = strtok(NULL, ";");
                }

                /* Step 8: Sort meanings by characteristic note. */
                sort_meanings_by_char_note(meanings, m);

                /*
                 * Step 9-10: Build final chord suggestions.
                 * Remove duplicates and keep only up to 3.
                 */
                int ucount = 0;
                for (int i = 0; i < m && ucount < 3; i++) {

                    char candidate[64];
                    if (meanings[i].type[0] == '\0') {
                        snprintf(candidate, sizeof(candidate), "%s",
                                 meanings[i].root_name);
                    } else {
                        snprintf(candidate, sizeof(candidate), "%s%s",
                                 meanings[i].root_name, meanings[i].type);
                    }

                    /* Check for duplicates. */
                    int dup = 0;
                    for (int j = 0; j < ucount; j++) {
                        if (strcmp(candidate, chord_names[j]) == 0) {
                            dup = 1;
                            break;
                        }
                    }

                    if (!dup) {
                        snprintf(chord_names[ucount], sizeof(chord_names[ucount]),
                                 "%s", candidate);
                        ucount++;
                    }
                }

                chord_count_out = ucount;

                free(copy);
                free(meanings);
            }
        }

        /*
         * Final printing (format exactly matches project PDF):
         *
         * <chord_number>: <start_time>
         * suggestion 1
         * suggestion 2   (optional)
         * suggestion 3   (optional)
         *
         */
        printf("%d: %.2f\n", idx + 1, start_times[idx]);
        for (int i = 0; i < chord_count_out; i++) {
            printf("%s\n", chord_names[i]);
        }
        printf("\n");

        free(totals);
        free(pc_totals);
        free(centroids);
    }

    free(vols);
    free(start_times);
    free(end_times);

    return 0;
}