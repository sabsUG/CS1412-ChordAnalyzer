#include <stdlib.h>
#include <string.h>
#include "trie.h"

int dchar_to_index(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c == 'A') return 10;
    if (c == 'B') return 11;
    return -1;
}

struct trie_node *trie_create_node() {
    struct trie_node *n = malloc(sizeof(struct trie_node));
    for (int i = 0; i < 12; i++)
        n->children[i] = NULL;
    n->meaning = NULL;
    return n;
}

void trie_insert(struct trie_node *root, const char *word, const char *meaning) {
    struct trie_node *curr = root;

    for (int i = 0; word[i] != '\0'; i++) {
        int idx = dchar_to_index(word[i]);
        if (curr->children[idx] == NULL)
            curr->children[idx] = trie_create_node();
        curr = curr->children[idx];
    }

    curr->meaning = malloc(strlen(meaning) + 1);
    strcpy(curr->meaning, meaning);
}

char *trie_search_longest_prefix(struct trie_node *root, const char *word) {
    struct trie_node *curr = root;
    char *best = NULL;

    for (int i = 0; word[i] != '\0'; i++) {
        int idx = dchar_to_index(word[i]);
        if (idx < 0 || curr->children[idx] == NULL)
            break;
        curr = curr->children[idx];

        if (curr->meaning != NULL)
            best = curr->meaning;   
    }

    return best;   
}
