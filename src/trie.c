#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "trie.h"

/* 0–9 -> 0–9, A -> 10, B -> 11, else -1 */
static int dchar_to_index(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c == 'A') return 10;
    if (c == 'B') return 11;
    return -1;
}

TrieNode *trie_create_node(void) {
    TrieNode *n = malloc(sizeof(TrieNode));
    if (!n) {
        fprintf(stderr, "Error: could not allocate trie node\n");
        return NULL;
    }
    for (int i = 0; i < 12; i++) {
        n->children[i] = NULL;
    }
    n->meaning = NULL;
    return n;
}

void trie_insert(TrieNode *root, const char *word, const char *meaning) {
    if (!root || !word || !meaning) return;

    TrieNode *curr = root;

    for (int i = 0; word[i] != '\0'; i++) {
        int idx = dchar_to_index(word[i]);
        if (idx < 0 || idx >= 12) {
            /* Should not happen for valid dictionary words. */
            fprintf(stderr, "Warning: invalid duodecimal char '%c' in word \"%s\"\n",
                    word[i], word);
            return;
        }

        if (!curr->children[idx]) {
            curr->children[idx] = trie_create_node();
            if (!curr->children[idx]) {
                return;
            }
        }
        curr = curr->children[idx];
    }

    /* Append meanings, always ending with ';' */
    size_t new_len = strlen(meaning) + 2;  /* meaning + ';' + '\0' */

    if (!curr->meaning) {
        curr->meaning = malloc(new_len);
        if (!curr->meaning) {
            fprintf(stderr, "Error: could not allocate meaning string\n");
            return;
        }
        strcpy(curr->meaning, meaning);
        strcat(curr->meaning, ";");
    } else {
        size_t old_len = strlen(curr->meaning);
        char *tmp = realloc(curr->meaning, old_len + new_len);
        if (!tmp) {
            fprintf(stderr, "Error: could not grow meaning string\n");
            return;
        }
        curr->meaning = tmp;
        strcat(curr->meaning, meaning);
        strcat(curr->meaning, ";");
    }
}

char *trie_search_longest_prefix(TrieNode *root, const char *word) {
    if (!root || !word) return NULL;

    TrieNode *curr = root;
    char *best = NULL;

    for (int i = 0; word[i] != '\0'; i++) {
        int idx = dchar_to_index(word[i]);
        if (idx < 0 || idx >= 12 || !curr->children[idx]) {
            break;
        }
        curr = curr->children[idx];
        if (curr->meaning) {
            best = curr->meaning;
        }
    }

    return best;
}