#include "dictADT.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "trie.h"

struct def {
    char *word;
    char *meanings;
};

struct dict_type {
    struct def *a; 
    int n_words;
    struct trie_node *trie;
};

Dict create_dict(char *filename) {
    char s[1000];

    FILE *fp = fopen(filename, "r");
    int c, n_lines = 0;

    while ((c = getc(fp)) != EOF)
        if (c == '\n')
            n_lines++;

    Dict d = malloc(sizeof(struct dict_type));
    d->n_words = n_lines;
    d->a = malloc(n_lines * sizeof(struct def));
    d->trie = trie_create_node();

    fseek(fp, 0, SEEK_SET);

    for (int i = 0; i < d->n_words; i++) {
        fgets(s, 1000, fp);

        // ↓ Find the dot that splits WORD. MEANINGS
        char *dot = strchr(s, '.');
        if (!dot) continue;

        *dot = '\0';  // terminate the word
        d->a[i].word = strdup(s);

        // ↓ MEANINGS = everything after the dot
        char *meaning = dot + 1;

        // Trim leading spaces
        while (*meaning == ' ' || *meaning == '\t') meaning++;

        // Trim trailing newline
        meaning[strcspn(meaning, "\r\n")] = '\0';

        d->a[i].meanings = strdup(meaning);

        // Insert into trie with full meanings
        trie_insert(d->trie, d->a[i].word, d->a[i].meanings);
    }

    fclose(fp);
    return d;
}

char *search(Dict d, char *word) {
    return trie_search_longest_prefix(d->trie, word);
}
