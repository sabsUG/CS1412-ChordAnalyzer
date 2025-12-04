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
    struct def *a; // Array of definitions
    int n_words; // How many defs in the dictionary
    struct trie_node *trie;
};

Dict create_dict(char *filename) {
    char s[1000];
    FILE *fp;
    fp = fopen(filename, "r");
    int c, n_lines = 0;
    while ((c = getc(fp)) != EOF)
        if (c == '\n')
            n_lines++;
    Dict d = malloc(sizeof(struct dict_type));
    d->n_words = n_lines;
    d->a = malloc(n_lines * sizeof(struct def));
    d->trie = trie_create_node();  
    fseek(fp, 0, SEEK_SET);
    for (int i=0; i<d->n_words; i++) { // Read each word:
        fgets(s, 1000, fp);
        char *t = strtok(s, ".");
        d->a[i].word = malloc(strlen(t) + 1);
        strcpy(d->a[i].word, t);
        //printf("%s\n",d->a[i].word);
        t = strtok(NULL, "\n") + 1;
        d->a[i].meanings = malloc(strlen(t) + 1);
        strcpy(d->a[i].meanings, t);
        //printf("%s\n",d->a[i].meanings);

        trie_insert(d->trie, d->a[i].word, d->a[i].meanings);

    }
    fclose(fp);
    return d;
}

char *search(Dict d, char *word) {
    return trie_search_longest_prefix(d->trie, word);
}